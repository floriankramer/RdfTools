// Copyright 2018 Florian Kramer

#include "NtParser.h"
#include <iostream>



const std::string& NtEntry::subject() {
  return _subject;
}

const std::string& NtEntry::predicate() {
  return _predicate;
}

const std::string& NtEntry::object() {
  return _object;
}

const bool NtEntry::isObjectLiteral() {
  return _isObjectLiteral;
}


NtEntry NtEntry::read(std::ifstream &in) {
  std::string line;
  std::getline(in, line);


  return NtEntry::fromString(line);
}

NtEntry NtEntry::fromString(const std::string &line) {
  NtEntry entry;
  entry._isObjectLiteral = false;
  entry._isValid = true;
  if (line.length() < 7 || line[0] == '#') {
    // the line is malformed
    entry._isValid = false;
    return entry;
  }
  size_t last = 0;
  size_t pos = 0;
  // If the current string being read is the subject, predicate or object
  int objectRead = 0;
  bool inString = false;
  bool escaped = false;
  while (pos < line.size()) {
    if (line[pos] == '\\') {
      escaped = !escaped;
    } else {
      if (!escaped && line[pos] == '"') {
        inString = !inString;
        if (objectRead == 2 && inString) {
          // the object apears to be a string literal
          entry._isObjectLiteral = true;
        }
      } else if (line[pos] == ' ' && !inString) {
        if (pos - last > 0) {
          switch (objectRead) {
          case 0:
            entry._subject = line.substr(last, pos - last);
            break;
          case 1:
            entry._predicate = line.substr(last, pos - last);
            break;
          case 2:
            entry._object = line.substr(last, pos  - last);
            break;
          }
          objectRead++;
          if (objectRead > 2) {
            break;
          }
        }
        last = pos + 1;
      }
      escaped = false;
    }
    pos++;
  }
  if (objectRead < 3) {
    entry._subject = "";
    entry._predicate = "";
    entry._object = "";
    entry._isValid = false;
  }
  return entry;
}

void NtEntry::write(std::ofstream &out) const {
  out << _subject << " "
      << _predicate << " "
      << _object << " ." << std::endl;
}

bool NtEntry::valid() {
  return _isValid;
}
