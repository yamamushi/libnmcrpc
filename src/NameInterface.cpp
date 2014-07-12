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

/* Source code for NameInterface.hpp.  */

#include "NameInterface.hpp"

#include <sstream>

namespace nmcrpc
{

/* ************************************************************************** */
/* High-level interface to Namecoin.  */

/**
 * Query for a name by string.  If the name is registered, this immediately
 * queries for the name's associated data.  If the name does not yet exist,
 * this still succeeds and returns a Name object that can be used to find
 * out that fact as well as register the name.
 * @param name The name to check.
 * @return The created name object.
 */
NameInterface::Name
NameInterface::queryName (const std::string& name)
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
NameInterface::Name
NameInterface::queryName (const std::string& ns, const std::string& name)
{
  std::ostringstream full;
  full << ns << "/" << name;
  return queryName (full.str ());
}

/* ************************************************************************** */
/* Name object.  */

/**
 * Construct the name.  This is meant to be used only
 * from inside NameInterface.  Outside users should use
 * NameInterface::queryName or other methods to obtain
 * name objects.
 * @param n The name's string.
 * @param nc NameInterface object.
 * @param rpc The RPC object to use for finding info about the name.
 */
NameInterface::Name::Name (const std::string& n, NameInterface& nc,
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
NameInterface::Name::ensureExists () const
{
  if (!ex)
    throw NameNotFound (name);
}

/**
 * Utility routine to split a name into namespace and trimmed parts.
 * @param name The full name.
 * @param ns Set to namespace part.
 * @param trimmed Set to trimmed part.
 * @return True if splitting was successful, false if there's no namespace.
 */
bool
NameInterface::Name::split (const std::string& name,
                                std::string& ns, std::string& trimmed)
{
  const std::string::size_type pos = name.find ('/');
  if (pos == std::string::npos)
    return false;

  ns = name.substr (0, pos);
  trimmed = name.substr (pos + 1);

  return true;
}

} // namespace nmcrpc
