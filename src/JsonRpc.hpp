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

#ifndef NMCRPC_JSONRPC_HPP
#define NMCRPC_JSONRPC_HPP

#include <stdexcept>
#include <string>

namespace nmcrpc
{

/* ************************************************************************** */
/* The JsonRpc class itself.  */

/**
 * JSON-RPC handling class.  It does the HTTP connection to the given server
 * as well as the JSON parsing/encoding, but doesn't care about the
 * Namecoin behind.
 */
class JsonRpc
{

public:

  /* Exception classes thrown for different errors.  */
  class Exception;
  class HttpError;
  class RpcError;

  /** Type of JSON data returned.  This is a Boost property-tree.  */
  // TODO: JSON data type.
  typedef int JsonData;

private:

  /** Connection host.  */
  std::string host;
  /** Connection port.  */
  unsigned port;
  /** Connection authentication username.  */
  std::string username;
  /** Connection authentication password.  */
  std::string password;

  /** The next ID to use for JSON-RPC queries.  */
  unsigned nextId;

  /**
   * Perform a HTTP query with JSON data.  However, this routine does not
   * know/care about JSON, it just sends the raw string and returns the
   * response body as string or throws if an error or an unaccepted
   * HTTP response code is detected.
   * @param query Query string to send.
   * @param responseCode Set to the HTTP response code.
   * @return The response body.
   * @throws Exception if some error occurs.
   */
  std::string queryHttp (const std::string& query, unsigned& responseCode);

public:

  /**
   * Construct for the given connection data.
   * @param h The connection host.
   * @param p The connection port.
   * @param u The user name.
   * @param pwd The password.
   */
  inline JsonRpc (const std::string& h, unsigned p,
                  const std::string& u, const std::string& pwd)
    : host(h), port(p), username(u), password(pwd), nextId(0)
  {
    // Nothing more to be done.
  }

  // We want no default constructor or copying.
  JsonRpc () = delete;
  JsonRpc (const JsonRpc& o) = delete;
  JsonRpc& operator= (const JsonRpc& o) = delete;

  /**
   * Decode JSON from a string.
   * @param str JSON string.
   * @returns The parsed JSON data.
   * @throws json_parser_error in case of parsing errors.
   */
  static inline JsonData
  decodeJson (const std::string& str)
  {
    // TODO: Implement.
    return -1;
  }

  /**
   * Encode JSON to a string.
   * @param data The JSON data.
   * @return The encoded JSON as string.
   */
  static inline std::string
  encodeJson (const JsonData& data)
  {
    // TODO: Implement.
    return "";
  }

  /**
   * Perform a JSON-RPC query with arbitrary parameter list.
   * @param method The method name to call.
   * @param params Iterable list of parameters to pass.
   * @return Result of the query as string.
   * @throws Exception in case of error.
   * @throws RpcError if the RPC call returns an error.
   */
  template<typename L>
    std::string executeRpcList (const std::string& method, const L& params);

  /* Utility methods to call RPC methods with small number of parameters.  */

  /*
  inline std::string
  executeRpc (const std::string& method)
  {
    std::vector<std::string> params;
    return executeRpcList (method, params);
  }

  template<typename T>
    inline std::string
    executeRpc (const std::string& method, const T& p1)
  {
    std::vector<std::string> params;
    params.push_back (boost::lexical_cast<std::string> (p1));
    return executeRpcList (method, params);
  }

  template<typename S, typename T>
    inline std::string
    executeRpc (const std::string& method, const S& p1, const T& p2)
  {
    std::vector<std::string> params;
    params.push_back (boost::lexical_cast<std::string> (p1));
    params.push_back (boost::lexical_cast<std::string> (p2));
    return executeRpcList (method, params);
  }

  template<typename R, typename S, typename T>
    inline std::string
    executeRpc (const std::string& method,
                const R& p1, const S& p2, const T& p3)
  {
    std::vector<std::string> params;
    params.push_back (boost::lexical_cast<std::string> (p1));
    params.push_back (boost::lexical_cast<std::string> (p2));
    params.push_back (boost::lexical_cast<std::string> (p3));
    return executeRpcList (method, params);
  }
  */

};

/* ************************************************************************** */
/* Exception classes.  */

/**
 * Unspecified exception during RPC call.
 */
class JsonRpc::Exception : public std::runtime_error
{

public:

  /**
   * Construct it just with an error message.
   * @param msg Error message.
   */
  explicit inline Exception (const std::string& msg)
    : std::runtime_error(msg)
  {
    // Nothing else to do.
  }

  /* No default constructor, but copying ok.  */
  Exception () = delete;
  Exception (const Exception& o) = default;
  Exception& operator= (const Exception& o) = default;

};

/**
 * HTTP connection error (returned with unaccepted HTTP response code).
 */
class JsonRpc::HttpError : public JsonRpc::Exception
{

private:

  /** The response code.  */
  unsigned code;

public:

  /**
   * Construct with the given HTTP code and a default message.
   * @param msg Error message.
   * @param c HTTP error code.
   */
  inline HttpError (const std::string& msg, unsigned c)
    : Exception(msg), code(c)
  {
    // Nothing else to do.
  }

  /* Default copying, no default constructor.  */
  HttpError () = delete;
  HttpError (const HttpError& o) = default;
  HttpError& operator= (const HttpError& o) = default;

  /**
   * Query HTTP response code.
   * @return The HTTP response code.
   */
  inline unsigned
  getResponseCode () const
  {
    return code;
  }

};

/**
 * Error returned by the RPC method call.
 */
class JsonRpc::RpcError : public JsonRpc::Exception
{

private:

  /** Error code.  */
  int code;

  /** Error message (from RPC).  */
  std::string message;

public:

  /**
   * Construct it given the JSON representation returned.
   * @param data JSON data returned by RPC.
   */
  explicit inline
  RpcError (const JsonData& data)
    : Exception("RPC returned an error response.")
  {
    // TODO: Implement based on JsonData.
    code = -1;
    message = "";
  }

  /* No default constructor but copying allowed.  */
  RpcError () = delete;
  RpcError (const RpcError& o) = default;
  RpcError& operator= (const RpcError& o) = default;

  /**
   * Get the error code.
   * @return Error code.
   */
  inline int
  getErrorCode () const
  {
    return code;
  }

  /**
   * Get the error message.
   * @return The error message.
   */
  inline const std::string&
  getErrorMessage () const
  {
    return message;
  }

};

/* ************************************************************************** */

/* Include template implementations.  */
#include "JsonRpc.tpp"

} // namespace nmcrpc

#endif /* Header guard.  */
