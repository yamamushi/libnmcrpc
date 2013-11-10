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

/* Utility program to update names.  */

#include "JsonRpc.hpp"
#include "NamecoinInterface.hpp"
#include "NameRegistration.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

using namespace nmcrpc;

/**
 * Display the help message.
 */
static void
displayHelp ()
{
  std::cerr << "Usage: nmupdate COMMAND [OPTIONS]"
            << std::endl << std::endl;
  std::cerr << "Possible commands:" << std::endl;
  std::cerr << "  * help: Display this message." << std::endl;
  std::cerr << "  * list: List owned names and their expirey counter."
            << std::endl;
  std::cerr << "  * update NAME [VAL]: Update NAME to VAL (or existing value)."
            << std::endl;
  std::cerr << "  * send NAME ADDR [VAL]: Update NAME and send to ADDR."
            << std::endl;
  std::cerr << "  * update-multi FILE [VAL]: Update all names in FILE"
            << std::endl
            << "                             to VAL or"
            << " their current value." << std::endl;
  std::cerr << "  * send-multi FILE ADDR [VAL]: Send all names in FILE to ADDR."
            << std::endl;
}

/**
 * Perform a name update operation on an array of names.
 * @param rpc Json RPC connection.
 * @param nc Namecoin high-level interface.
 * @param names Array of names to update.
 * @param hasVal Whether to set the value or leave it.
 * @param val The value, is ignored if !hasVal.
 */
static void
performUpdate (JsonRpc& rpc, NamecoinInterface& nc,
               const std::vector<std::string>& names,
               bool hasVal, const std::string& val)
{
  for (const auto& nm : names)
    {
      std::cout << "Updating " << nm << ": ";
      NamecoinInterface::Name name = nc.queryName (nm);

      NameUpdate updater (rpc, nc, name);
      if (hasVal)
        updater.setValue (val);
      const std::string txid = updater.execute ();

      std::cout << txid << std::endl;
    }
}

/**
 * Main routine with the usual interface.
 */
int
main (int argc, char** argv)
{
  try
    {
      if (argc < 2)
        {
          displayHelp ();
          return EXIT_FAILURE;
        }
      const std::string command = argv[1];

      if (command == "help")
        {
          displayHelp ();
          return EXIT_SUCCESS;
        }

      RpcSettings settings;
      settings.readDefaultConfig ();
      JsonRpc rpc(settings);
      NamecoinInterface nc(rpc);

      if (command == "list")
        {
          std::vector<NamecoinInterface::Name> names;
          const auto addName = [&names] (const NamecoinInterface::Name& nm)
            {
              names.push_back (nm);
            };
          nc.forMyNames (addName);

          const auto comp = [] (const NamecoinInterface::Name& a,
                                const NamecoinInterface::Name& b) -> bool
            {
              int diff = a.getExpireCounter () - b.getExpireCounter ();
              if (diff != 0)
                return (diff > 0);
              return a.getName () < b.getName ();
            };
          std::sort (names.begin (), names.end (), comp);

          for (const auto& el : names)
            {
              std::cout.width (30);
              std::cout << el.getName () << ": "
                        << el.getExpireCounter () << std::endl;
            }
        }
      else
        {
          std::string passphrase;
          if (nc.needWalletPassphrase ())
            {
              std::cout << "Enter wallet passphrase: ";
              std::getline (std::cin, passphrase);
            }
          NamecoinInterface::WalletUnlocker unlock(nc, passphrase);

          std::vector<std::string> names;

          if (command == "update")
            {
              if (argc < 3 || argc > 4)
                throw std::runtime_error ("Expected: nmupdate update"
                                          " NAME [VAL]");

              const std::string name = argv[2];
              std::string val;
              if (argc == 4)
                val = argv[3];

              names.push_back (name);
              performUpdate (rpc, nc, names, argc == 4, val);
            }
          else
            throw std::runtime_error ("Unknown command '" + command + "'.");
        }
    }
  catch (const JsonRpc::RpcError& exc)
    {
      std::cerr << "JSON-RPC error:" << std::endl;
      std::cerr << exc.getErrorMessage () << std::endl;
      return EXIT_FAILURE;
    }
  catch (const std::exception& exc)
    {
      std::cerr << "Error: " << exc.what () << std::endl;
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}