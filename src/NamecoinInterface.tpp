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

/* Template implementation for NamecoinInterface.hpp.  */

/* ************************************************************************** */
/* High-level interface to Namecoin.  */

/**
 * Query for all user-owned names in the wallet (according to name_list but
 * filtering out names that have been sent away) and execute some call-back
 * on them.
 * @param cb Call-back routine.
 */
template<typename T>
  void
  NamecoinInterface::forMyNames (T cb)
{
  const JsonRpc::JsonData res = rpc.executeRpc ("name_list");
  assert (res.isArray ());
  for (const JsonRpc::JsonData& val : res)
    {
      NamecoinInterface::Name nm = queryName (val["name"].asString ());
      if (nm.getAddress ().isMine ())
        cb (nm);
    }
}
