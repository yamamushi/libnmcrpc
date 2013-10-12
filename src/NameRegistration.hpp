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

#ifndef NMCRPC_NAMEREGISTRATION_HPP
#define NMCRPC_NAMEREGISTRATION_HPP

#include "JsonRpc.hpp"
#include "NamecoinInterface.hpp"

#include <iostream>
#include <list>
#include <stdexcept>
#include <string>

namespace nmcrpc
{

/* ************************************************************************** */
/* Handle registration of a name.  */

/**
 * This class represents the registration process of a single name.  This
 * includes handling of the persistent data needed in order to do first
 * name_new and then name_firstupdate after some time.  This class
 * stores the necessary data when the reserve() function is called, and
 * can later hand it to name_firstupdate in activate().  You have to call
 * both functions in that order and with a sufficient delay between.
 *
 * By using the stream operators << and >>, the current state can be
 * saved and loaded.  This is useful in order to save the state possibly
 * between multiple runs of the program.
 */
class NameRegistration
{

public:

  /* Exceptions.  */
  class NameAlreadyReserved;

  /** Possible "states" of a registration process.  */
  enum State
  {
    /** Registration is not yet started (uninitialised object).  */
    NOT_STARTED,

    /** name_new has beed called but not yet name_firstupdate.  */
    REGISTERED,

    /** We're finished, including name_firstupdate.  */
    ACTIVATED
  };

private:

  /** Underlying RPC connection.  */
  JsonRpc& rpc;

  /** Current state.  */
  State state;

  /** The name we want to register, for simplicity with saving as string.  */
  std::string name;
  /** Value we want to set with firstupdate.  */
  std::string value;

  /** Random value of name_new.  */
  std::string rand;
  /** Txid of name_new.  */
  std::string tx;

  /** Txid of name_firstupdate.  */
  std::string txActivation;

  /**
   * Number of confirmations we want on the name_new transaction before
   * performing name_firstupdate.
   */
  static const unsigned FIRSTUPDATE_DELAY;

  /**
   * Utility routine to check for the number of confirmations a transaction
   * already has.  It throws the appropriate RpcError if the transaction ID
   * is not found.
   * @param txid The transaction id to check for.
   * @return Number of confirmations the transaction has.
   */
  unsigned getNumberOfConfirmations (const std::string& txid) const;

public:

  /**
   * Construct it with the given RPC connection.
   * @param r The RPC connection.
   */
  explicit inline NameRegistration (JsonRpc& r)
    : rpc(r), state(NOT_STARTED)
  {
    // Nothing more to be done.
  }

  // No default constructor, copying is ok.
  NameRegistration () = delete;
  NameRegistration (const NameRegistration&) = default;
  NameRegistration& operator= (const NameRegistration&) = default;

  /**
   * Start registration of a name with issuing the corresponding name_new
   * transaction.
   * @param nm The name to register, as Name object.
   * @throws std::runtime_error if we're not in NOT_STARTED state.
   * @throws NameAlreadyReserved if the name already exists.
   */
  void registerName (const NamecoinInterface::Name& nm);

  /**
   * Set the value to use with firstupdate as string.
   * @param val The value we want to set.
   * @throws std::runtime_error if we're not in REGISTERED state.
   */
  inline void setValue (const std::string& val)
  {
    if (state != REGISTERED)
      throw std::runtime_error ("Can setValue() only in REGISTERED state.");

    value = val;
  }

  /**
   * Set the value to use with firstupdate as JSON object.
   * @param val The JSON value we want to set.
   * @throws std::runtime_error if we're not in REGISTERED state.
   */
  inline void
  setValue (const JsonRpc::JsonData& val)
  {
    setValue (JsonRpc::encodeJson (val));
  }

  /**
   * Return the current state.
   * @return The current state.
   */
  inline State
  getState () const
  {
    return state;
  }

  /**
   * Return the current name.
   * @return The name being registered as string.
   */
  inline const std::string&
  getName () const
  {
    return name;
  }

  /**
   * Check whether we can already perform a firstupdate transaction.
   * @return True iff we're in REGISTERED state and enough time has passed.
   */
  bool canActivate () const;

  /**
   * Activate the name, which issues the firstupdate transaction.
   * @throws std::runtime_error if this is not (yet) possible.
   */
  void activate ();

  /**
   * Check whether the registration is finished, which includes a confirmation
   * of the firstupdate transaction.
   * @return True iff the name was activated and the tx confirmed.
   */
  bool isFinished () const;

  /**
   * For registered but not yet finished registration processes, save the
   * state necessary to later perform firstupdate to a stream.
   * @param out The output stream.
   * @param obj The NameRegistration object to save.
   * @return The stream.
   * @throws std::runtime_error if the name is not REGISTERED/ACTIVATED state.
   */
  friend std::ostream& operator<< (std::ostream& out,
                                   const NameRegistration& obj);

  /**
   * Load state of a registration process from the stream, so that we can
   * finish with firstupdate.
   * @param in The input stream.
   * @param obj The NameRegistration object to load into.
   * @return The stream.
   * @throws std::runtime_error/JsonParseError if no valid data can be found.
   */
  friend std::istream& operator>> (std::istream& in, NameRegistration& obj);

};

/* ************************************************************************** */
/* Manage multiple name registration processes.  */

/**
 * Handle multiple name registration processes.  This is basically an array
 * of NameRegistration objects that allows to save/restore and update
 * all of them at once.
 */
class RegistrationManager
{

private:

  /** The RPC connection to use.  */
  JsonRpc& rpc;

  /** Type used internally to keep the list of names.  */
  typedef std::list<NameRegistration*> nameListT;

  /** Store the name registration processes.  */
  nameListT names;

  /**
   * Clear all elements, freeing the memory properly.
   */
  void clear ();

  /**
   * Template to use for const and non-const iterators.  They are based on
   * the underlying iterators of the names list, but they dereference also
   * the pointer already.
   */
  template<typename Base, typename Val>
    class Iterator
  {

  private:

    friend class RegistrationManager;

    typedef Iterator<Base, Val> selfT;

    /** Iterator backing this one.  */
    Base iter;

    /**
     * Construct it given a base iterator.  This is private so that it can
     * only be used by RegistrationManager itself.
     * @param i The base iterator.
     */
    explicit inline Iterator (const Base& i)
      : iter (i)
    {
      // Nothing else to do.
    }

  public:

    // Copying and default constructor.
    Iterator () = default;
    Iterator (const selfT&) = default;
    selfT& operator= (const selfT&) = default;

    /* Compare for equality and inequality.  */

    friend inline bool
    operator== (const selfT& a, const selfT& b)
    {
      return a.iter == b.iter;
    }

    friend inline bool
    operator!= (const selfT& a, const selfT& b)
    {
      return a.iter != b.iter;
    }

    /* Increment the iterator.  */

    inline selfT&
    operator++ ()
    {
      ++iter;
      return *this;
    }
    
    inline selfT
    operator++ (int)
    {
      selfT res = *this;
      ++iter;
      return res;
    }

    /**
     * Dereference the iterator, which returns the NameRegistration as
     * reference and not pointer.
     * @return The currently pointed to NameRegistration object.
     */
    inline Val&
    operator* () const
    {
      return **iter;
    }

  };

public:

  /* Define iterators.  */
  typedef Iterator<nameListT::iterator, NameRegistration> iterator;
  typedef Iterator<nameListT::const_iterator,
                   const NameRegistration> const_iterator;

  /**
   * Construct it empty.
   * @param r The RPC interface to use.
   */
  inline RegistrationManager (JsonRpc& r)
    : rpc(r), names()
  {
    // Nothing more to do.
  }

  // No copying.
  RegistrationManager () = delete;
  RegistrationManager (const RegistrationManager&) = delete;
  RegistrationManager& operator= (const RegistrationManager&) = delete;

  /**
   * Destroy it safely.
   */
  ~RegistrationManager ();

  /**
   * Start registration for a new name.  The process object is returned so that
   * the value can be set as desired.
   * @param nm The name to register.
   * @return The NameRegistration object created and inserted.
   * @throws NameAlreadyReserved if the name already exists.
   */
  NameRegistration& registerName (const NamecoinInterface::Name& nm);

  /**
   * Try to update all processes, which activates names where it is possible.
   */
  void update ();

  /**
   * Purge finished names from the list.
   * @return Number of elements purged.
   */
  unsigned cleanUp ();

  /* Get iterators.  */

  inline iterator
  begin ()
  {
    return iterator (names.begin ());
  }

  inline const_iterator
  begin () const
  {
    return const_iterator (names.begin ());
  }

  inline iterator
  end ()
  {
    return iterator (names.end ());
  }

  inline const_iterator
  end () const
  {
    return const_iterator (names.end ());
  }

  /**
   * Write out all states to the stream.
   * @param out The output stream.
   * @param obj The RegistrationManager object to save.
   * @return The stream.
   */
  friend std::ostream& operator<< (std::ostream& out,
                                   const RegistrationManager& obj);

  /**
   * Load all states from the stream, replacing all objects that
   * are currently in the list (if any).
   * @param in The input stream.
   * @param obj The object to load into.
   * @return The stream.
   * @throws std::runtime_error/JsonParseError if no valid data can be found.
   */
  friend std::istream& operator>> (std::istream& in, RegistrationManager& obj);

};

/* ************************************************************************** */
/* Exception classes.  */

/**
 * Thrown when a name to be registered is already reserved.
 */
class NameRegistration::NameAlreadyReserved : public std::runtime_error
{

private:

  /** The name that is already reserved.  */
  std::string name;

public:

  /**
   * Construct it given the name that is already taken.
   * @param n The already reserved name.
   */
  explicit inline NameAlreadyReserved (const std::string& n)
    : std::runtime_error("Name is already reserved: " + n), name(n)
  {
    // Nothing else to do.
  }

  /* No default constructor, but copying ok.  */
  NameAlreadyReserved () = delete;
  NameAlreadyReserved (const NameAlreadyReserved&) = default;
  NameAlreadyReserved& operator= (const NameAlreadyReserved&) = default;

  /**
   * Get the name of this error.
   * @return The name that is already reserved.
   */
  inline const std::string&
  getName () const
  {
    return name;
  }

};

} // namespace nmcrpc

#endif /* Header guard.  */
