/*  Namecoin RPC library.
 *  Copyright (C) 2013-2014  Daniel Kraft <d@domob.eu>
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

#ifndef NMCRPC_COININTERFACE_HPP
#define NMCRPC_COININTERFACE_HPP

#include "JsonRpc.hpp"

#include <cassert>
#include <stdexcept>
#include <string>

namespace nmcrpc
{

/* ************************************************************************** */
/* High-level interface to the core wallet.  */

/**
 * Allow high-level interfacing to a coin daemon via an underlying RPC
 * connection.  This is the part that is independent of names, just balances
 * and addresses.  It can be used for Bitcoin or other coins as well as
 * Namecoin.
 */
class CoinInterface
{

public:

  /* Exceptions.  */
  class NoPrivateKey;
  class UnlockFailure;

  /* Other child classes.  */
  class Address;
  class WalletUnlocker;

protected:

  /** Underlying RPC connection.  */
  JsonRpc& rpc;

private:

  /**
   * The number of seconds we want to temporarily unlock the wallet in case
   * we need access to a private key.
   */
  static const unsigned UNLOCK_SECONDS;

  // Disable copying and default constructor.
#ifndef CXX_11
  CoinInterface ();
  CoinInterface (const CoinInterface&);
  CoinInterface& operator= (const CoinInterface&);
#endif /* !CXX_11  */

public:

  /**
   * Construct it with the given RPC connection.
   * @param r The RPC connection.
   */
  explicit inline CoinInterface (JsonRpc& r)
    : rpc(r)
  {
    // Nothing more to be done.
  }

  // We want no default constructor or copying.
#ifdef CXX_11
  CoinInterface () = delete;
  CoinInterface (const CoinInterface&) = delete;
  CoinInterface& operator= (const CoinInterface&) = delete;
#endif /* CXX_11?  */

  /**
   * Run a simple test command and return whether the connection seems to
   * be up and running.  It also produces a message that can be shown, which
   * either includes the running version or an error string.
   * @param msg Set this to the message string.
   * @return True iff the connection seems to be fine.
   */
  bool testConnection (std::string& msg);

  /**
   * Run test command and discard message string.
   * @return True iff the connection seems to be fine.
   */
  inline bool
  testConnection ()
  {
    std::string msg;
    return testConnection (msg);
  }

  /**
   * Query for an address by string.  This immediately checks whether the
   * address is valid and owned by the user, so that this information can be
   * encapsulated into the returned object.
   * @param addr The address to check.
   * @return The created address object.
   */
  Address queryAddress (const std::string& addr);

  /**
   * Create a new address (as per "getnewaddress") and return it.
   * @return Newly created address.
   */
  Address createAddress ();

  /**
   * Query for the number of confirmations a transaction has.
   * @param txid The transaction id to check for.
   * @return Number of confirmations the transaction has.
   * @throws JsonRpc::RpcError if the tx is not found.
   */
  unsigned getNumberOfConfirmations (const std::string& txid);

  /**
   * Check whether the wallet needs to be unlocked or not.  This routine is
   * used to decide whether we need to ask for a passphrase or not before
   * using WalletUnlocker.
   * @return True iff we need a passphrase.
   */
  bool needWalletPassphrase ();

};

/* ************************************************************************** */
/* Address object.  */

/**
 * Encapsulate a Namecoin address.
 */
class CoinInterface::Address
{

private:

  friend class CoinInterface;

  /** The namecoin interface to be used for RPC calls (message signing).  */
  JsonRpc* rpc;

  /** The address as string.  */
  std::string addr;

  /** Whether the address is valid.  */
  bool valid;
  /** Whether the address is owned by the user.  */
  bool mine;

  /**
   * Construct the address.  This is meant to be used only
   * from inside CoinInterface.  Outside users should use
   * CoinInterface::queryAddress or other methods to obtain
   * address objects.
   * @param r The namecoin interface to use.
   * @param a The address as string.
   */
  Address (JsonRpc& r, const std::string& a);

public:

  /**
   * Default constructor, initialises to "".  This is here for convenience
   * so that variables of type Address can be declared.
   */
  inline Address ()
    : rpc(nullptr), addr(""), valid(false), mine(false)
  {
    // Nothing more to do.
  }

  // Copying is ok.
#ifdef CXX_11
  Address (const Address&) = default;
  Address& operator= (const Address&) = default;
#endif /* CXX_11?  */

  /**
   * Get the address as string.
   * @return The address as string.
   */
  inline const std::string&
  getAddress () const
  {
    return addr;
  }

  /**
   * Get whether or not the address is valid.
   * @return True iff this is a valid address.
   */
  inline bool
  isValid () const
  {
    return valid;
  }

  /**
   * Get whether or not the address is owned by the user.
   * @return True iff the user has the address' private key.
   */
  inline bool
  isMine () const
  {
    return mine;
  }

  /**
   * Check a message signature against this address.  If this address
   * is invalid, false is returned.
   * @param msg The message that should be signed.
   * @param sig The message's signature.
   * @return True iff the signature matches the message.
   */
  bool verifySignature (const std::string& msg, const std::string& sig) const;

  /**
   * Sign a message with this address.  This may throw if the address
   * is invalid, the wallet locked or the private key is not owned.
   * @param msg The message that should be signed.
   * @return The message's signature.
   * @throws NoPrivateKey if this address is not owned.
   * @throws std::runtime_error if the address is invalid or the wallet locked.
   */
  std::string signMessage (const std::string& msg) const;

};

/* ************************************************************************** */
/* Wallet unlocker.  */

/**
 * Unlock the wallet temporarily (during the scope of the object).  RAII
 * ensures that the wallet is locked (if it was indeed unlocked) again
 * at the latest when we go out of scope.
 */
class CoinInterface::WalletUnlocker
{

private:

  friend class CoinInterface;

  /** The RPC object to use.  */
  JsonRpc& rpc;
  /** High-level interface to use.  */
  CoinInterface& nc;

  /** Whether we actually unlocked the wallet.  */
  bool unlocked;

  // Disable copying and default constructor.
#ifndef CXX_11
  WalletUnlocker ();
  WalletUnlocker (const WalletUnlocker&);
  WalletUnlocker& operator= (const WalletUnlocker&);
#endif /* !CXX_11  */

public:

  /**
   * Construct it, not yet unlocking.
   * @param n The CoinInterface to use.
   */
  explicit WalletUnlocker (CoinInterface& n);

  // No default constructor or copying.
#ifdef CXX_11
  WalletUnlocker () = delete;
  WalletUnlocker (const WalletUnlocker&) = delete;
  WalletUnlocker& operator= (const WalletUnlocker&) = delete;
#endif /* CXX_11?  */

  /**
   * Lock the wallet on destruct.
   */
  ~WalletUnlocker ();

  /**
   * Perform the unlock (if necessary).  The passphrase must be correct if the
   * wallet is actually locked, and can be anything else.
   * @param passphrase Passphrase to use for unlocking.
   * @throws UnlockFailure if the passphrase is wrong.
   */
  void unlock (const std::string& passphrase);

};

/* ************************************************************************** */
/* Exception classes.  */

/**
 * Thrown when an action is to be performed for which the private key
 * is missing from the wallet.
 */
class CoinInterface::NoPrivateKey : public std::runtime_error
{

private:

  // Disable default constructor.
#ifndef CXX_11
  NoPrivateKey ();
#endif /* !CXX_11  */

public:

  /**
   * Construct it given the error message.
   * @param msg The error message.
   */
  explicit inline NoPrivateKey (const std::string& msg)
    : std::runtime_error(msg)
  {
    // Nothing else to do.
  }

  /* No default constructor, but copying ok.  */
#ifdef CXX_11
  NoPrivateKey () = delete;
  NoPrivateKey (const NoPrivateKey&) = default;
  NoPrivateKey& operator= (const NoPrivateKey&) = default;
#endif /* CXX_11?  */

};

/**
 * Thrown in case of failure to unlock the wallet (because of a wrong
 * passphrase, presumably).
 */
class CoinInterface::UnlockFailure : public std::runtime_error
{

private:

  // Disable default constructor.
#ifndef CXX_11
  UnlockFailure ();
#endif /* !CXX_11  */

public:

  /**
   * Construct it given the error message.
   * @param msg The error message.
   */
  explicit inline UnlockFailure (const std::string& msg)
    : std::runtime_error(msg)
  {
    // Nothing else to do.
  }

  /* No default constructor, but copying ok.  */
#ifdef CXX_11
  UnlockFailure () = delete;
  UnlockFailure (const UnlockFailure&) = default;
  UnlockFailure& operator= (const UnlockFailure&) = default;
#endif /* CXX_11?  */

};

} // namespace nmcrpc

#endif /* Header guard.  */
