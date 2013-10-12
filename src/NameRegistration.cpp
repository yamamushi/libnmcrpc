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

#include "NameRegistration.hpp"

#include <cassert>
#include <memory>
#include <sstream>

namespace nmcrpc
{

/* ************************************************************************** */
/* Handle registration of a name.  */

/**
 * Number of confirmations we want on the name_new transaction before
 * performing name_firstupdate.
 */
const unsigned NameRegistration::FIRSTUPDATE_DELAY = 12;

/**
 * Utility routine to check for the number of confirmations a transaction
 * already has.  It throws the appropriate RpcError if the transaction ID
 * is not found.
 * @param txid The transaction id to check for.
 * @return Number of confirmations the transaction has.
 */
unsigned
NameRegistration::getNumberOfConfirmations (const std::string& txid) const
{
  const JsonRpc::JsonData res = rpc.executeRpc ("gettransaction", txid);
  assert (res.isObject ());
  return res["confirmations"].asInt ();
}

/**
 * Start registration of a name with issuing the corresponding name_new
 * transaction.
 * @param nm The name to register, as Name object.
 * @throws std::runtime_error if we're not in NOT_STARTED state.
 * @throws NameAlreadyReserved if the name already exists.
 */
void
NameRegistration::registerName (const NamecoinInterface::Name& nm)
{
  if (state != NOT_STARTED)
    throw std::runtime_error ("Can registerName() only in NOT_STARTED state.");
  if (nm.exists () && !nm.isExpired ())
    throw NameAlreadyReserved (nm.getName ());

  name = nm.getName ();
  const JsonRpc::JsonData res = rpc.executeRpc ("name_new", name);
  assert (res.isArray () && res.size () == 2);
  rand = res[1].asString ();
  tx = res[0].asString ();

  /* Set default value, can be changed now.  */
  value = "";

  /* Update state as last action, so that it is not changed if some action
     above throws.  */
  state = REGISTERED;
}

/**
 * Check whether we can already perform a firstupdate transaction.
 * @return True iff we're in REGISTERED state and enough time has passed.
 */
bool
NameRegistration::canActivate () const
{
  if (state != REGISTERED)
    return false;

  return (getNumberOfConfirmations (tx) >= FIRSTUPDATE_DELAY);
}

/**
 * Activate the name, which issues the firstupdate transaction.
 * @throws std::runtime_error if this is not (yet) possible.
 */
void
NameRegistration::activate ()
{
  if (state != REGISTERED)
    throw std::runtime_error ("Can activate() only in REGISTERED state.");
  if (!canActivate ())
    throw std::runtime_error ("Can't yet activate, please wait longer.");

  JsonRpc::JsonData args(Json::arrayValue);
  args.append (name);
  args.append (rand);
  args.append (tx);
  args.append (value);
  const JsonRpc::JsonData res = rpc.executeRpcArray ("name_firstupdate", args);

  assert (res.isString ());
  txActivation = res.asString ();
  state = ACTIVATED;
}

/**
 * Check whether the registration is finished, which includes a confirmation
 * of the firstupdate transaction.
 * @return True iff the name was activated and the tx confirmed.
 */
bool
NameRegistration::isFinished () const
{
  if (state != ACTIVATED)
    return false;

  return (getNumberOfConfirmations (txActivation) > 0);
}

/**
 * For registered but not yet activated registration processes, save the
 * state necessary to later perform firstupdate to a stream.
 * @param out The output stream.
 * @param obj The NameRegistration object to save.
 * @return The stream.
 * @throws std::runtime_error if the name is not REGISTERED state.
 */
std::ostream&
operator<< (std::ostream& out, const NameRegistration& obj)
{
  JsonRpc::JsonData outVal(Json::objectValue);
  outVal["type"] = "NameRegistration";
  outVal["version"] = 1;
  outVal["name"] = obj.name;

  switch (obj.state)
    {
    case NameRegistration::REGISTERED:
      outVal["state"] = "registered";
      outVal["value"] = obj.value;
      outVal["rand"] = obj.rand;
      outVal["tx"] = obj.tx;
      break;

    case NameRegistration::ACTIVATED:
      outVal["state"] = "activated";
      outVal["txActivation"] = obj.txActivation;
      break;

    default:
      throw std::runtime_error ("Wrong state for saving of NameRegistration.");
    }

  out << JsonRpc::encodeJson (outVal);
  return out;
}

/**
 * Load state of a registration process from the stream, so that we can
 * finish with firstupdate.
 * @param in The input stream.
 * @param obj The NameRegistration object to load into.
 * @return The stream.
 * @throws std::runtime_error/JsonParseError if no valid data can be found.
 */
std::istream&
operator>> (std::istream& in, NameRegistration& obj)
{
  const JsonRpc::JsonData inVal = JsonRpc::readJson (in);

  if (inVal["type"].asString () != "NameRegistration"
      || inVal["version"].asInt () != 1)
    throw std::runtime_error ("Wrong JSON object found, expected"
                              " version 1 NameRegistration.");

  obj.name = inVal["name"].asString ();

  const std::string state = inVal["state"].asString ();
  if (state == "registered")
    {
      obj.state = NameRegistration::REGISTERED;
      obj.value = inVal["value"].asString ();
      obj.rand = inVal["rand"].asString ();
      obj.tx = inVal["tx"].asString ();
    }
  else if (state == "activated")
    {
      obj.state = NameRegistration::ACTIVATED;
      obj.txActivation = inVal["txActivation"].asString ();
    }
  else
    throw std::runtime_error ("Invalid state found in the JSON data"
                              " of NameRegistration.");

  return in;
}

/* ************************************************************************** */
/* Manage multiple name registration processes.  */

/**
 * Destroy it safely.
 */
RegistrationManager::~RegistrationManager ()
{
  clear ();
}

/**
 * Clear all elements, freeing the memory properly.
 */
void
RegistrationManager::clear ()
{
  for (auto ptr : names)
    delete ptr;
  names.clear ();
}

/**
 * Start registration for a new name.  The process object is returned so that
 * the value can be set as desired.
 * @param nm The name to register.
 * @return The NameRegistration object created and inserted.
 * @throws NameAlreadyReserved if the name already exists.
 */
NameRegistration&
RegistrationManager::registerName (const NamecoinInterface::Name& nm)
{
  std::unique_ptr<NameRegistration> ptr;

  ptr.reset (new NameRegistration (rpc));
  ptr->registerName (nm);

  names.push_back (ptr.release ());
  return *names.back ();
}

/**
 * Try to update all processes, which activates names where it is possible.
 */
void
RegistrationManager::update ()
{
  for (auto& nm : *this)
    if (nm.canActivate ())
      nm.activate ();
}

/**
 * Purge finished names from the list.
 */
void
RegistrationManager::cleanUp ()
{
  nameListT::iterator i = names.begin ();
  while (i != names.end ())
    {
      if ((*i)->isFinished ())
        i = names.erase (i);
      else
        ++i;
    }
}

/**
 * Write out all states to the stream.
 * @param out The output stream.
 * @param obj The RegistrationManager object to save.
 * @return The stream.
 */
std::ostream&
operator<< (std::ostream& out, const RegistrationManager& obj)
{
  JsonRpc::JsonData outVal(Json::objectValue);
  outVal["type"] = "RegistrationManager";
  outVal["version"] = 1;

  JsonRpc::JsonData arr(Json::arrayValue);
  for (const auto& nm : obj)
    {
      std::ostringstream val;
      val << nm;
      arr.append (val.str ());
    }
  outVal["elements"] = arr;

  out << JsonRpc::encodeJson (outVal);
  return out;
}

/**
 * Load all states from the stream, replacing all objects that
 * are currently in the list (if any).
 * @param in The input stream.
 * @param obj The object to load into.
 * @return The stream.
 * @throws std::runtime_error/JsonParseError if no valid data can be found.
 */
std::istream&
operator>> (std::istream& in, RegistrationManager& obj)
{
  const JsonRpc::JsonData inVal = JsonRpc::readJson (in);

  if (inVal["type"].asString () != "RegistrationManager"
      || inVal["version"].asInt () != 1)
    throw std::runtime_error ("Wrong JSON object found, expected"
                              " version 1 RegistrationManager.");

  const JsonRpc::JsonData elements = inVal["elements"];
  if (!elements.isArray ())
    throw std::runtime_error ("Invalid JSON for RegistrationManager.");

  obj.clear ();
  for (const JsonRpc::JsonData el : elements)
    {
      std::istringstream val(el.asString ());
      std::unique_ptr<NameRegistration> ptr;

      ptr.reset (new NameRegistration (obj.rpc));
      val >> *ptr;
      obj.names.push_back (ptr.release ());
    }

  return in;
}

} // namespace nmcrpc