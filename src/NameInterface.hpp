/*  Namecoin RPC library.
 *  Copyright (C) 2013-2014  Daniel Kraft <d@domob.eu>
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

#ifndef NMCRPC_NAMEINTERFACE_HPP
#define NMCRPC_NAMEINTERFACE_HPP

#include "CoinInterface.hpp"
#include "JsonRpc.hpp"

#include <cassert>
#include <stdexcept>
#include <string>

namespace nmcrpc
{

/* ************************************************************************** */
/* High-level interface to Namecoin.  */

/**
 * Extend the generic coin interface with high-level access to name
 * functionality.
 */
class NameInterface : public CoinInterface
{

public:

  /* Exceptions.  */
  class NameNotFound;

  /* Other child classes.  */
  class Name;

private:

  // Disable copying and default constructor.
#ifndef CXX_11
  NameInterface ();
  NameInterface (const NameInterface&);
  NameInterface& operator= (const NameInterface&);
#endif /* !CXX_11  */

public:

  /**
   * Construct it with the given RPC connection.
   * @param r The RPC connection.
   */
  explicit inline NameInterface (JsonRpc& r)
    : CoinInterface(r)
  {
    // Nothing more to be done.
  }

  // We want no default constructor or copying.
#ifdef CXX_11
  NameInterface () = delete;
  NameInterface (const NameInterface&) = delete;
  NameInterface& operator= (const NameInterface&) = delete;
#endif /* CXX_11?  */

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

  /**
   * Query for all user-owned names in the wallet (according to name_list but
   * filtering out names that have been sent away) and execute some call-back
   * on them.
   * @param cb Call-back routine.
   */
  template<typename T>
    void forMyNames (T cb);

  /**
   * Query for all names in the index (according to name_scan) and execute
   * some call-back on them.
   * @param cb Call-back routine.
   */
  template<typename T>
    void forAllNames (T cb);

};

/* ************************************************************************** */
/* Name object.  */

/**
 * Encapsulate a Namecoin name.
 */
class NameInterface::Name
{

private:

  friend class NameInterface;

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
   * from inside NameInterface.  Outside users should use
   * NameInterface::queryName or other methods to obtain
   * name objects.
   * @param n The name's string.
   * @param nc NameInterface object.
   * @param rpc The RPC object to use for finding info about the name.
   */
  Name (const std::string& n, NameInterface& nc, JsonRpc& rpc);

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
#ifdef CXX_11
  Name (const Name&) = default;
  Name& operator= (const Name&) = default;
#endif /* CXX_11?  */

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
    return (data["expired"].isInt () && (data["expired"].asInt () != 0));
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

  /**
   * Utility routine to split a name into namespace and trimmed parts.
   * @param name The full name.
   * @param ns Set to namespace part.
   * @param trimmed Set to trimmed part.
   * @return True if splitting was successful, false if there's no namespace.
   */
  static bool split (const std::string& name,
                     std::string& ns, std::string& trimmed);

};

/* ************************************************************************** */
/* Exception classes.  */

/**
 * Thrown when a name that is not yet registered is asked for.
 */
class NameInterface::NameNotFound : public std::runtime_error
{

private:

  /** The name that was not found.  */
  std::string name;

  // Disable default constructor.
#ifndef CXX_11
  NameNotFound ();
#endif /* !CXX_11  */

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
#ifdef CXX_11
  NameNotFound () = delete;
  NameNotFound (const NameNotFound&) = default;
  NameNotFound& operator= (const NameNotFound&) = default;
#endif /* CXX_11?  */

  // Specify throw() explicitly.
#ifndef CXX_11
  inline ~NameNotFound () throw ()
  {}
#endif /* !CXX_11  */

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

/* ************************************************************************** */

/* Include template implementations.  */
#include "NameInterface.tpp"

} // namespace nmcrpc

#endif /* Header guard.  */
