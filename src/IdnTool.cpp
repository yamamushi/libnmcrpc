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

/* Source code for IdnTool.hpp.  */

#include "IdnTool.hpp"

#include <idna.h>
#include <stringprep.h>

#include <cassert>
#include <clocale>
#include <cstring>
#include <sstream>
#include <stdexcept>

namespace nmcrpc
{

/**
 * Default constructor.  Currently, we have no settings and thus
 * need no parameters.  However, it calls setlocale() to properly set
 * up the locale settings from the environment.
 */
IdnTool::IdnTool ()
{
  std::setlocale (LC_ALL, "");
}

/**
 * Decode IDN to native locale string.  This routine handles the full string
 * and does not split off a namespace prefix.
 * @param in Input string, encoded as IDN.
 * @return Decoded output string.
 */
std::string
IdnTool::decodeFull (const std::string& in)
{
  char* buf = nullptr;

  const auto flags = static_cast<Idna_flags> (0);
  Idna_rc rc;
  rc = static_cast<Idna_rc> (idna_to_unicode_lzlz (in.c_str (), &buf, flags));

  if (rc != IDNA_SUCCESS)
    {
      std::free (buf);
      std::ostringstream msg;
      msg << "IDNA decoding failed: " << idna_strerror (rc);
      throw std::runtime_error (msg.str ());
    }
  assert (buf);

  const std::string res(buf);
  std::free (buf);

  return res;
}

/**
 * Encode a native string to IDN.  Handle the full string and do not split
 * off a namespace prefix.
 * @param in Input string in native locale encoding.
 * @return IDN encoded output string.
 */
std::string
IdnTool::encodeFull (const std::string& in)
{
  char* buf = nullptr;

  const auto flags = static_cast<Idna_flags> (0);
  Idna_rc rc;
  rc = static_cast<Idna_rc> (idna_to_ascii_lz (in.c_str (), &buf, flags));

  if (rc != IDNA_SUCCESS)
    {
      std::free (buf);
      std::ostringstream msg;
      msg << "IDNA encoding failed: " << idna_strerror (rc);
      throw std::runtime_error (msg.str ());
    }
  assert (buf);

  const std::string res(buf);
  std::free (buf);

  return res;
}

} // namespace nmcrpc
