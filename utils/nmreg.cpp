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

/* Utility program to register Namecoin names using libnmcrpc.  */

#include "JsonRpc.hpp"
#include "NamecoinInterface.hpp"
#include "NameRegistration.hpp"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace nmcrpc;

/**
 * Display the help message.
 */
static void
displayHelp ()
{
  std::cerr << "Usage: nmreg COMMAND [FILE] [OPTIONS]"
            << std::endl << std::endl;
  std::cerr << "Possible commands:" << std::endl;
  std::cerr << "  * help: Display this message." << std::endl;
  std::cerr << "  * info: Show information about the state in FILE."
            << std::endl;
  std::cerr << "  * update: Update all processes in FILE if possible."
            << std::endl;
  std::cerr << "  * clear: Remove already finished processes from FILE."
            << std::endl;
  std::cerr << "  * register: Register the given name with the given value."
            << std::endl;
  std::cerr << "  * multi: Register all names in the file with the given value."
            << std::endl;
}

/**
 * Perform the 'info' action, displaying info about the state.
 * @param reg The registration manager object to use.
 */
static void
doInfo (const RegistrationManager& reg)
{
  std::cout << "Names in registration:" << std::endl << std::endl;
  for (const auto& nm : reg)
    {
      std::cout << nm.getName () << ": ";
      switch (nm.getState ())
        {
        case NameRegistration::NOT_STARTED:
          assert (false);
          break;

        case NameRegistration::REGISTERED:
          std::cout << "registered, can";
          if (!nm.canActivate ())
            std::cout << " not";
          std::cout << " activate";
          break;

        case NameRegistration::ACTIVATED:
          std::cout << "activated";
          if (nm.isFinished ())
            std::cout << " and finished";
          break;

        default:
          assert (false);
          break;
        }
      std::cout << std::endl;
    }
}

/**
 * Perform registration action.
 * @param reg The registration manager object to use.
 * @param nc The Namecoin interface to use.
 * @param name The name to register.
 * @param val The value to set.
 */
static void
doRegister (RegistrationManager& reg, NamecoinInterface& nc,
            const std::string& name, const std::string& val)
{
  const NamecoinInterface::Name nm = nc.queryName (name);
  NameRegistration& cur = reg.registerName (nm);
  cur.setValue (val);

  std::cout << "Started registration of " << name << "." << std::endl;
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
          throw std::runtime_error ("Usage: nmreg COMMAND [FILE] [OPTIONS]");
        }
      const std::string command = argv[1];

      if (command == "help")
        {
          displayHelp ();
          return EXIT_SUCCESS;
        }

      if (argc < 3)
        throw std::runtime_error ("Need FILE argument.");
      const std::string stateFile = argv[2];

      RpcSettings settings;
      settings.readDefaultConfig ();
      JsonRpc rpc(settings);
      NamecoinInterface nc(rpc);
      RegistrationManager reg(rpc);

      std::ifstream fileIn(stateFile);
      if (fileIn)
        {
          std::cout << "Reading old state." << std::endl;
          fileIn >> reg;
          fileIn.close ();
        }
      else
        std::cout << "No old state to read, intialising empty." << std::endl;

      if (command == "info")
        doInfo (reg);
      else if (command == "clear")
        {
          const unsigned cleaned = reg.cleanUp ();
          std::cout << "Removed " << cleaned << " finished names." << std::endl;
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

          if (command == "update")
            {
              reg.update ();
              std::cout << "Updated all processes." << std::endl;
            }
          else if (command == "register")
            {
              if (argc != 5)
                throw std::runtime_error ("Expected: nmreg register"
                                          " FILE NAME VALUE");

              const std::string name = argv[3];
              const std::string val = argv[4];

              doRegister (reg, nc, name, val);
            }
          else if (command == "multi") 
            {
              if (argc != 5)
                throw std::runtime_error ("Expected: nmreg multi"
                                          " FILE LIST-FILE VALUE");

              const std::string listFile = argv[3];
              const std::string val = argv[4];

              std::ifstream listIn(listFile);
              if (!listIn)
                throw std::runtime_error ("Could not read list of names.");

              while (listIn)
                {
                  std::string line;
                  std::getline (listIn, line);

                  if (!line.empty ())
                    doRegister (reg, nc, line, val);
                }
            }
          else
            throw std::runtime_error ("Unknown command '" + command + "'.");
        }

      std::ofstream fileOut(stateFile);
      fileOut << reg;
      fileOut.close ();
      std::cout << "Wrote new state." << std::endl;
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