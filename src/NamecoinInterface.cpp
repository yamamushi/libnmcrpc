/*  Namecoin RPC library.
 *  Copyright (C) 2013  Daniel Kraft <d@domob.eu>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  See the distributed file COPYING for additional permissions in addition
 *  to those of the GNU Affero General Public License.
 */

/* Source code for NamecoinInterface.hpp.  */

#include "NamecoinInterface.hpp"

#include <ctime>
#include <sstream>

namespace nmcrpc
{

/* ************************************************************************** */
/* High-level interface to Namecoin.  */

const unsigned NamecoinInterface::UNLOCK_SECONDS = 10;

/**
 * Query for an address by string.  This immediately checks whether the
 * address is valid and owned by the user, so that this information can be
 * encapsulated into the returned object.
 * @param addr The address to check.
 * @return The created address object.
 */
NamecoinInterface::Address
NamecoinInterface::queryAddress (const std::string& addr)
{
  return Address (rpc, addr);
}

/**
 * Query for a name by string.  If the name is registered, this immediately
 * queries for the name's associated data.  If the name does not yet exist,
 * this still succeeds and returns a Name object that can be used to find
 * out that fact as well as register the name.
 * @param name The name to check.
 * @return The created name object.
 */
NamecoinInterface::Name
NamecoinInterface::queryName (const std::string& name)
{
  return Name (name, *this, rpc);
}

/**
 * Query for a name by namespace and name.
 * @see queryName (const std::string&)
 * @param ns The namespace.
 * @param name The (namespace-less) name.
 * @return The created name object.
 */
NamecoinInterface::Name
NamecoinInterface::queryName (const std::string& ns, const std::string& name)
{
  std::ostringstream full;
  full << ns << "/" << name;
  return queryName (full.str ());
}

/**
 * Check whether the wallet needs to be unlocked or not.  This routine is
 * used to decide whether we need to ask for a passphrase or not before
 * using WalletUnlocker.
 * @return True iff we need a passphrase.
 */
bool
NamecoinInterface::needWalletPassphrase ()
{
  const JsonRpc::JsonData res = rpc.executeRpc ("getinfo");

  if (res["unlocked_until"].isNull ())
    return false;

  const int until = res["unlocked_until"].asInt ();
  return (until < std::time (nullptr) + UNLOCK_SECONDS);
}

/* ************************************************************************** */
/* Address object.  */

/**
 * Construct the address.  This is meant to be used only
 * from inside NamecoinInterface.  Outside users should use
 * NamecoinInterface::queryAddress or other methods to obtain
 * address objects.
 * @param r The namecoin interface to use.
 * @param a The address as string.
 */
NamecoinInterface::Address::Address (JsonRpc& r, const std::string& a)
  : rpc(&r), addr(a)
{
  const JsonRpc::JsonData res = rpc->executeRpc ("validateaddress", addr);
  valid = res["isvalid"].asBool ();
  mine = false;
  if (valid)
    mine = res["ismine"].asBool ();
}

/**
 * Check a message signature against this address.  If this address
 * is invalid, false is returned.
 * @param msg The message that should be signed.
 * @param sig The message's signature.
 * @return True iff the signature matches the message.
 */
bool
NamecoinInterface::Address::verifySignature (const std::string& msg,
                                             const std::string& sig) const
{
  if (!valid)
    return false;

  try
    {
      JsonRpc::JsonData res = rpc->executeRpc ("verifymessage", addr, sig, msg);
      return (res.isBool () && res.asBool ());
    }
  catch (const JsonRpc::RpcError& exc)
    {
      // Malformed base64?
      if (exc.getErrorCode () == -5)
        return false;
      throw exc;
    }
}

/**
 * Sign a message with this address.  This may throw if the address
 * is invalid, the wallet locked or the private key is not owned.
 * @param msg The message that should be signed.
 * @return The message's signature.
 * @throws NoPrivateKey if this address is not owned.
 * @throws std::runtime_error if the address is invalid or the wallet locked.
 */
std::string
NamecoinInterface::Address::signMessage (const std::string& msg) const
{
  if (!valid)
    throw std::runtime_error ("Can't sign with invalid address.");

  try
    {
      const JsonRpc::JsonData res = rpc->executeRpc ("signmessage", addr, msg);
      return res.asString ();
    }
  catch (const JsonRpc::RpcError& exc)
    {
      switch (exc.getErrorCode ())
        {
        case -13:
          throw std::runtime_error ("Need to unlock the wallet first.");

        case -3:
          {
            std::ostringstream msg;
            msg << "You don't have the private key of " << addr << " in order"
                << " to sign messages with that address.";
            throw NoPrivateKey (msg.str ());
          }

        default:
          throw exc;
        }
    }
}

/* ************************************************************************** */
/* Name object.  */

/**
 * Construct the name.  This is meant to be used only
 * from inside NamecoinInterface.  Outside users should use
 * NamecoinInterface::queryName or other methods to obtain
 * name objects.
 * @param n The name's string.
 * @param nc NamecoinInterface object.
 * @param rpc The RPC object to use for finding info about the name.
 */
NamecoinInterface::Name::Name (const std::string& n, NamecoinInterface& nc,
                               JsonRpc& rpc)
  : initialised(true), name(n)
{
  try
    {
      data = rpc.executeRpc ("name_show", n);
      ex = true;
      addr = nc.queryAddress (data["address"].asString ());
    }
  catch (const JsonRpc::RpcError& exc)
    {
      if (exc.getErrorCode () == -4)
        {
          ex = false;
          return;
        }
      throw exc;
    }
}

/**
 * Ensure that this object has status "exists".
 * @throws NameNotFound if the name doesn't yet exist.
 */
void
NamecoinInterface::Name::ensureExists () const
{
  if (!ex)
    throw NameNotFound (name);
}

/* ************************************************************************** */
/* Wallet unlocker.  */

/**
 * Construct it, which unlocks the wallet (if necessary).  The passphrase
 * must be correct if unlock is needed, and can be anything if it is not.
 * @param nc The NamecoinInterface to use.
 * @param passphrase Passphrase to use for unlocking.
 * @throws UnlockFailure if the passphrase is wrong.
 */
NamecoinInterface::WalletUnlocker::WalletUnlocker
  (NamecoinInterface& nc, const std::string& passphrase)
  : rpc(nc.rpc)
{
  unlocked = nc.needWalletPassphrase ();
  if (unlocked)
    {
      /* Ensure the wallet is indeed locked before we send the passphrase.
         It could be the case that it is unlocked although for too short
         a time, then lock it now.  */
      rpc.executeRpc ("walletlock");
      try
        {
          rpc.executeRpc ("walletpassphrase", passphrase, UNLOCK_SECONDS);
        }
      catch (const JsonRpc::RpcError& exc)
        {
          if (exc.getErrorCode () == -14)
            throw UnlockFailure ("Wrong wallet passphrase.");
          throw exc;
        }
    }
}

/**
 * Lock the wallet on destruct.
 */
NamecoinInterface::WalletUnlocker::~WalletUnlocker ()
{
  if (unlocked)
    rpc.executeRpc ("walletlock");
}

} // namespace nmcrpc
