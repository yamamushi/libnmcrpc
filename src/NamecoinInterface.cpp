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

namespace nmcrpc
{

/* ************************************************************************** */
/* High-level interface to Namecoin.  */

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
  JsonRpc::JsonData res = rpc.executeRpc ("validateaddress", addr);
  const bool valid = res["isvalid"].asBool ();
  bool mine = false;
  if (valid)
    mine = res["ismine"].asBool ();

  return Address (addr, valid, mine);
}

} // namespace nmcrpc
