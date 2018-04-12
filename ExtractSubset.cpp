// Copyright 2017 Florian Kramer

#include <getopt.h>
#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unordered_set>

#include "NtParser.h"

void printHelp(char* exeName) {
  printf("Usage: %s [Options] <infile> <outfile>\n", exeName);
  printf("Reads the nt file and writes a subset of the entries to outfile.\n\n");
  printf("Options\n");
  printf("%-20s\t%s\n", "-d, --depth", "How many relations to follow.");
  printf("%-20s\t%s\n", "-s, --source", "The name of the source entity.");
}

struct Settings {
  std::string sourceFile;
  std::string targetFile;
  int depth = 1;
  std::string sourceEntity = "<http://www.wikidata.org/entity/Q26>";
};

Settings parseSettings(int argc, char **argv) {
  Settings settings;

  option options[] = {
    {"depth", required_argument, NULL, 'd'},
    {"source", required_argument, NULL, 's'},
    {NULL, 0, NULL, 0}
  };
  char c;
  while ((c = getopt_long(argc, argv, "d:s:", options, NULL)) != -1) {
    switch (c) {
    case 'd':
      settings.depth = atoi(optarg);
      break;
    case 's':
      settings.sourceEntity = std::string(optarg);
      break;
    default:
      printf("%s is an unrecognized option.\n", argv[optind]);
      printHelp(argv[0]);
      exit(1);
      break;
    }
  }
  if (argc - optind < 2) {
    printf("Not enough arguments\n");
    printHelp(argv[0]);
    exit(1);
  }
  settings.sourceFile = std::string(argv[optind]);
  settings.targetFile = std::string(argv[optind + 1]);
  return settings;
}

int main(int argc, char **argv) {
  Settings settings = parseSettings(argc, argv);
  // open the in and out files
  std::ifstream in;
  in.open(settings.sourceFile);
  if (!in) {
    fprintf(stderr, "Unable to open file %s\n", settings.sourceFile.c_str());
    return 1;
  }
  std::ofstream out;
  out.open(settings.targetFile);
  if (!out) {
    fprintf(stderr, "Unable to open file %s\n", settings.sourceFile.c_str());
    return 1;
  }
  std::unordered_set<std::string> knownEntities;
  std::unordered_set<std::string> knownPredicates;
  knownEntities.insert(settings.sourceEntity);

  // iterate over all entries and copy those that are of interest
  size_t numLinesParsed = 0;
  size_t numLinesMax = 0;
  size_t lastNumLinesPrinted = 0;

  printf("Extracting enitities and predicates...\n");
  while (!in.eof()) {
    NtEntry entry = NtEntry::read(in);
    if (entry.subject() == settings.sourceEntity) {
      if (!entry.isObjectLiteral()) {
        // If the object is not a literal add it to the known entities
        // and write it.
        knownEntities.insert(entry.object());
        knownPredicates.insert(entry.predicate());
        entry.write(out);
      }
    }
    numLinesParsed++;
    if (numLinesParsed - lastNumLinesPrinted > 10000) {
      printf("\rLines parsed: %u", numLinesParsed);
      lastNumLinesPrinted = numLinesParsed;
    }
  }
  numLinesMax = numLinesParsed;
  printf("Done\n");

  // Iterate over all entities a second time and write the names of all
  // entities and predicates that are mentioned in any entry written during the
  // first pass
  printf("Extracting names...\n");
  // clear the eof bit
  in.clear();
  in.seekg(0, std::ios::beg);
  numLinesParsed = 0;
  while (!in.eof()) {
    NtEntry entry = NtEntry::read(in);
    if (entry.predicate() == "<http://schema.org/name>"
        && (knownEntities.find(entry.subject()) != knownEntities.end()
            || knownPredicates.find(entry.subject()) != knownPredicates.end())) {
      entry.write(out);
    }
    numLinesParsed++;
    if (numLinesParsed - lastNumLinesPrinted > 10000) {
      printf("\rLines parsed: %u / %u", numLinesParsed, numLinesMax);
      lastNumLinesPrinted = numLinesParsed;
    }
  }
  printf("Done\n");

  in.close();
  out.close();
}
