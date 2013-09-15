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

/* Test program for name registration.  */

#include "JsonRpc.hpp"
#include "NamecoinInterface.hpp"
#include "NameRegistration.hpp"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace nmcrpc;

int
main (int argc, char** argv)
{
  if (argc != 2)
    {
      std::cerr << "Usage: registerName STATE-FILE" << std::endl;
      return EXIT_FAILURE;
    }
  const std::string fileName = argv[1];

  RpcSettings settings;
  settings.readDefaultConfig ();
  JsonRpc rpc(settings);
  NamecoinInterface nc(rpc);
  NameRegistration reg(rpc);

  std::ifstream fileIn(fileName);
  if (fileIn)
    {
      std::cout << "Found state file, reading and trying to firstupdate."
                << std::endl;
      fileIn >> reg;
      fileIn.close ();
      
      if (reg.canActivate ())
        {
          reg.activate ();
          std::cout << "Activated the name." << std::endl;
        }
      else
        std::cout << "Please wait longer." << std::endl;
    }
  else
    {
      std::string name;
      std::cout << "Name to register: ";
      std::cin >> name;
      reg.registerName (nc.queryName (name));

      std::string value;
      std::cout << "Value to set: ";
      std::cin >> value;
      reg.setValue (value);

      std::ofstream fileOut(fileName);
      fileOut << reg;
      fileOut.close ();
    }

  return EXIT_SUCCESS;
}
