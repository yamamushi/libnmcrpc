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

/* Source code for JsonRpc.hpp.  */

#include "JsonRpc.hpp"

namespace nmcrpc
{

/* ************************************************************************** */
/* The JsonRpc class itself.  */

/**
 * Perform a HTTP query with JSON data.  However, this routine does not
 * know/care about JSON, it just sends the raw string and returns the
 * response body as string or throws if an error or an unaccepted
 * HTTP response code is detected.
 * @param query Query string to send.
 * @param responseCode Set to the HTTP response code.
 * @return The response body.
 * @throws Exception if some error occurs.
 */
std::string
JsonRpc::queryHttp (const std::string& query, unsigned& responseCode)
{
  /* FIXME: Implement.  */
  return "";
}

} // namespace nmcrpc
