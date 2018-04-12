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
  printf("%-20s\t%s\n", "-l, --lang", "Adds another language to the whitelist.");
}

struct Settings {
  std::string sourceFile;
  std::string targetFile;
  std::unordered_set<std::string> languages;
};

Settings parseSettings(int argc, char **argv) {
  Settings settings;

  option options[] = {
    {"lang", required_argument, NULL, 'l'},
    {NULL, 0, NULL, 0}
  };
  char c;
  while ((c = getopt_long(argc, argv, "l:", options, NULL)) != -1) {
    switch (c) {
    case 'l':
      settings.languages.insert(std::string(optarg));
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
  // iterate over all entries and copy those that are of interest
  in.seekg(0, std::ios_base::end);
  size_t filesize = in.tellg();
  in.seekg(0, std::ios_base::beg);
  size_t bytesRead = 0;
  size_t lastPercentagePrinted = 0;
  size_t lineNum = 0;
  size_t numTriplesRead = 0;
  size_t numTriplesWritten = 0;
  std::string line;
  printf("\nProgress: %d%%", lastPercentagePrinted);
  fflush(stdout);
  while (!in.eof()) {
    std::getline(in, line);
    lineNum++;
    bytesRead += line.size();
    if ((bytesRead * 100) / filesize > lastPercentagePrinted) {
      lastPercentagePrinted = (bytesRead * 100) / filesize;
      printf("\rProgress: %d%%", lastPercentagePrinted);
      fflush(stdout);
    }
    NtEntry entry = NtEntry::fromString(line);
    if (!entry.valid()) {
      std::cerr <<  std::endl << "The line " << lineNum << " : " << line
                << "does not contain a valid triple" << std::endl;
    } else {
      numTriplesRead++;
      if (entry.isObjectLiteral() && entry.object().size() > 3) {
        size_t atPos = entry.object().rfind('@');
        if (atPos != std::string::npos && atPos + 1 < entry.object().size()) {
          std::string langTag = entry.object().substr(atPos + 1);
          if (settings.languages.find(langTag) != settings.languages.end()) {
            // on;y write the triple if the language is known
            entry.write(out);
            numTriplesWritten++;
          }
        } else {
          entry.write(out);
          numTriplesWritten++;
        }
      } else {
        entry.write(out);
        numTriplesWritten++;
      }
    }
  }
  in.close();
  out.close();
  printf("Done\n\n");
  std::cout << "Read " << lineNum << " lines." << std::endl;
  std::cout << "Parsed " << numTriplesRead << " triples" << std::endl;
  std::cout << "of which " << numTriplesWritten << " where written out again"
            << std::endl;
}
