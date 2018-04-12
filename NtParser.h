// Copyright 2018 Florian Kramer

#ifndef NTFILTERS_NTPARSER_H_
#define NTFILTERS_NTPARSER_H_

#include <string>
#include <fstream>


/**
 * @brief Represents a single line of the tf file
 */
class NtEntry {
 public:
  /**
   * @brief Reads a single line from in and interprets it as an entity. Assumes
   *        the delimiters are always just a single space
   * @param in The std::ifstream to read from
   */
  static NtEntry read(std::ifstream &in);
  static NtEntry fromString(const std::string &line);

  /**
   * @brief Writes the given entry into out in the tn format.
   * @param out
   * @param entry
   */
  void write(std::ofstream &out) const;

  /**
   * @brief valid
   * @return Whether or not the triple represents a valid entity
   */
  bool valid();

  const std::string& subject();
  const std::string& predicate();
  const std::string& object();
  const bool isObjectLiteral();

 private:
  std::string _subject;
  std::string _predicate;
  std::string _object;
  bool _isObjectLiteral;
  bool _isValid;
};



#endif  // NTFILTERS_NTPARSER_H_
