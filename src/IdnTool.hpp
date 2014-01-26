/*  Namecoin RPC library.
 *  Copyright (C) 2014  Daniel Kraft <d@domob.eu>
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

#ifndef NMCRPC_IDNTOOL_HPP
#define NMCRPC_IDNTOOL_HPP

#include <cstddef>
#include <string>

namespace nmcrpc
{

/**
 * Class with basic IDN encoding and decoding capabilities.
 */
class IdnTool
{

public:

  /**
   * Default constructor.  Currently, we have no settings and thus
   * need no parameters.  However, it calls setlocale() to properly set
   * up the locale settings from the environment.
   */
  IdnTool ();

  // Allow copying.
  IdnTool (const IdnTool&) = default;
  IdnTool& operator= (const IdnTool&) = default;

  /**
   * Decode IDN to native locale string.  This routine handles the full string
   * and does not split off a namespace prefix.
   * @param in Input string, encoded as IDN.
   * @return Decoded output string.
   */
  std::string decodeFull (const std::string& in);

  /**
   * Encode a native string to IDN.  Handle the full string and do not split
   * off a namespace prefix.
   * @param in Input string in native locale encoding.
   * @return IDN encoded output string.
   */
  std::string encodeFull (const std::string& in);

};

} // namespace nmcrpc

#endif /* Header guard.  */
