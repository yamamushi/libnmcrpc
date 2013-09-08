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
#include <stdexcept>
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

  /**
   * Query for a name by string.  If the name is registered, this immediately
   * queries for the name's associated data.  If the name does not yet exist,
   * this still succeeds and returns a Name object that can be used to find
   * out that fact as well as register the name.
   * @param name The name to check.
   * @return The created name object.
   */
  Name queryName (const std::string& name);

  /**
   * Query for a name by namespace and name.
   * @see queryName (const std::string&)
   * @param ns The namespace.
   * @param name The (namespace-less) name.
   * @return The created name object.
   */
  Name queryName (const std::string& ns, const std::string& name);

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
   * Construct the address.  This is meant to be used only
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
/* Name object.  */

/**
 * Encapsulate a Namecoin name.
 */
class NamecoinInterface::Name
{

private:

  friend class NamecoinInterface;

  /**
   * Whether or not this is a default-constructed object.  Those can't be
   * used for anything.
   */
  bool initialised;

  /** The name's string.  */
  std::string name;

  /** Whether or not the name is already registered.  */
  bool ex;

  /** The address holding the name.  */
  Address addr;

  /** The name's JSON data, which name_show returns.  */
  JsonRpc::JsonData data;

  /**
   * Construct the name.  This is meant to be used only
   * from inside NamecoinInterface.  Outside users should use
   * NamecoinInterface::queryName or other methods to obtain
   * name objects.
   * @param n The name's string.
   * @param nc NamecoinInterface object.
   * @param rpc The RPC object to use for finding info about the name.
   */
  Name (const std::string& n, NamecoinInterface& nc, JsonRpc& rpc);

  /**
   * Ensure that this object is initialised and not default-constructed.
   * @throws std::runtime_error if it is default-constructed.
   */
  inline void
  ensureInitialised () const
  {
    if (!initialised)
      throw std::runtime_error ("Name is not yet initialised.");
  }

  /**
   * Ensure that this object has status "exists".
   * @throws NameNotFound if the name doesn't yet exist.
   */
  void ensureExists () const;

public:

  /**
   * Default constructor, marks object as "invalid".  This is here for
   * convenience so that variables of type Name can be declared, but they
   * can't be used for anything until they have been assigned to.
   */
  inline Name ()
    : initialised(false)
  {
    // Nothing more to do.
  }

  // Copying is ok.
  Name (const Name&) = default;
  Name& operator= (const Name&) = default;

  /**
   * Get the name as string.
   * @return The name as string.
   */
  inline const std::string&
  getName () const
  {
    ensureInitialised ();
    return name;
  }

  /**
   * Get the address holding the name.
   * @return The name's address.
   * @throws NameNotFound if the name doesn't yet exist.
   */
  inline const Address&
  getAddress () const
  {
    ensureExists ();
    return addr;
  }

  /**
   * Get whether or not the name exists.
   * @return True iff this name already exists.
   */
  inline bool
  exists () const
  {
    ensureInitialised ();
    return ex;
  }

  /**
   * Get the name's full JSON info as per name_show.
   * @return This name's full JSON info.
   * @throws NameNotFound if the name doesn't yet exist.
   */
  inline const JsonRpc::JsonData&
  getFullData () const
  {
    ensureExists ();
    return data;
  }

  /**
   * Get the name's value as string.
   * @return This name's value as string.
   * @throws NameNotFound if the name doesn't yet exist.
   */
  inline const std::string
  getStringValue () const
  {
    ensureExists ();
    return data["value"].asString ();
  }

  /**
   * Get the name's value as JSON object.
   * @return This name's value as JSON object.
   * @throws NameNotFound if the name doesn't yet exist.
   * @throws JsonRpc::JsonParseError if JSON parsing fails.
   */
  inline JsonRpc::JsonData
  getJsonValue () const
  {
    return JsonRpc::decodeJson (getStringValue ());
  }

  /**
   * Return whether the name is expired.
   * @return True iff this name exists but is expired.
   * @throws NameNotFound if the name doesn't yet exist.
   */
  inline bool
  isExpired () const
  {
    ensureExists ();
    return (data["expired"].isBool () && data["expired"].asBool ());
  }

  /**
   * Return number of blocks until the name expires.
   * @return The number of blocks until the name expires.  Might be negative.
   * @throws NameNotFound if the name doesn't yet exist.
   */
  inline int
  getExpireCounter () const
  {
    ensureExists ();
    return data["expires_in"].asInt ();
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
