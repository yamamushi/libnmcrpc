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

/* Test program for the JSON-RPC interface.  */

#include "JsonRpc.hpp"

#include <cstdlib>
#include <iostream>

using namespace nmcrpc;

int
main (int argc, char** argv)
{
  JsonRpc rpc ("localhost", 8336, "daniel", "password");

  unsigned code;
  std::cout << rpc.queryHttp ("foobar", code) << "\n\n";

  std::cout << "Code: " << code << "\n";

  return EXIT_SUCCESS;
}
