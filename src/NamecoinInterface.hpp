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

#ifndef NMCRPC_NAMECOININTERFACE_HPP
#define NMCRPC_NAMECOININTERFACE_HPP

#include "JsonRpc.hpp"

#include <cassert>
#include <string>

namespace nmcrpc
{

/* ************************************************************************** */
/* High-level interface to Namecoin.  */

/**
 * Allow high-level interfacing to Namecoin via an underlying RPC connection.
 */
class NamecoinInterface
{

public:

  /* Exceptions.  */
  class NameNotFound;
  class NoPrivateKey;

  /* Other child classes.  */
  class Address;
  class Name;

private:

  /** Underlying RPC connection.  */
  JsonRpc& rpc;

public:

  /**
   * Construct it with the given RPC connection.
   * @param r The RPC connection.
   */
  explicit inline NamecoinInterface (JsonRpc& r)
    : rpc(r)
  {
    // Nothing more to be done.
  }

  // We want no default constructor or copying.
  NamecoinInterface () = delete;
  NamecoinInterface (const NamecoinInterface&) = delete;
  NamecoinInterface& operator= (const NamecoinInterface&) = delete;

  /**
   * Query for an address by string.  This immediately checks whether the
   * address is valid and owned by the user, so that this information can be
   * encapsulated into the returned object.
   * @param addr The address to check.
   * @return The created address object.
   */
  Address queryAddress (const std::string& addr);

};

/* ************************************************************************** */
/* Address object.  */

/**
 * Encapsulate a Namecoin address.
 */
class NamecoinInterface::Address
{

private:

  friend class NamecoinInterface;

  /** The address as string.  */
  std::string addr;

  /** Whether the address is valid.  */
  bool valid;
  /** Whether the address is owned by the user.  */
  bool mine;

  /**
   * Construct the address This is meant to be used only
   * from inside NamecoinInterface.  Outside users should use
   * NamecoinInterface::queryAddress or other methods to obtain
   * address objects.
   * @param a The address as string.
   * @param v Valid?
   * @param m Mine?
   */
  inline Address (const std::string& a, bool v, bool m)
    : addr(a), valid(v), mine(m)
  {
    assert (!(mine && !valid));
  }

public:

  /**
   * Default constructor, initialises to "".  This is here for convenience
   * so that variables of type Address can be declared.
   */
  inline Address ()
    : addr(""), valid(false), mine(false)
  {
    // Nothing more to do.
  }

  // Copying is ok.
  Address (const Address&) = default;
  Address& operator= (const Address&) = default;

  /**
   * Get the address as string.
   * @return The address as string.
   */
  inline const std::string&
  getAddress () const
  {
    return addr;
  }

  /**
   * Get whether or not the address is valid.
   * @return True iff this is a valid address.
   */
  inline bool
  isValid () const
  {
    return valid;
  }

  /**
   * Get whether or not the address is owned by the user.
   * @return True iff the user has the address' private key.
   */
  inline bool
  isMine () const
  {
    return mine;
  }

};

/* ************************************************************************** */
/* Exception classes.  */

/**
 * Thrown when a name that is not yet registered is asked for.
 */
class NamecoinInterface::NameNotFound : public std::runtime_error
{

private:

  /** The name that was not found.  */
  std::string name;

public:

  /**
   * Construct it given the name that is not found.
   * @param n The undefined name.
   */
  explicit inline NameNotFound (const std::string& n)
    : std::runtime_error("Name not found: " + n), name(n)
  {
    // Nothing else to do.
  }

  /* No default constructor, but copying ok.  */
  NameNotFound () = delete;
  NameNotFound (const NameNotFound&) = default;
  NameNotFound& operator= (const NameNotFound&) = default;

  /**
   * Get the name of this error.
   * @return The name that was not found.
   */
  inline const std::string&
  getName () const
  {
    return name;
  }

};

/**
 * Thrown when an action is to be performed for which the private key
 * is missing from the wallet.
 */
class NamecoinInterface::NoPrivateKey : public std::runtime_error
{

public:

  /**
   * Construct it given the error message.
   * @param msg The error message.
   */
  explicit inline NoPrivateKey (const std::string& msg)
    : std::runtime_error(msg)
  {
    // Nothing else to do.
  }

  /* No default constructor, but copying ok.  */
  NoPrivateKey () = delete;
  NoPrivateKey (const NoPrivateKey&) = default;
  NoPrivateKey& operator= (const NoPrivateKey&) = default;

};

} // namespace nmcrpc

#endif /* Header guard.  */
