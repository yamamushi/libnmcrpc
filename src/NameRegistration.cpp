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
  if (obj.state != NameRegistration::REGISTERED)
    throw std::runtime_error ("Can only write state in REGISTERED state.");

  JsonRpc::JsonData outVal(Json::objectValue);
  outVal["type"] = "NameRegistration";
  outVal["version"] = 1;
  outVal["name"] = obj.name;
  outVal["value"] = obj.value;
  outVal["rand"] = obj.rand;
  outVal["tx"] = obj.tx;

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

  obj.state = NameRegistration::REGISTERED;
  obj.name = inVal["name"].asString ();
  obj.value = inVal["value"].asString ();
  obj.rand = inVal["rand"].asString ();
  obj.tx = inVal["tx"].asString ();

  return in;
}

} // namespace nmcrpc
