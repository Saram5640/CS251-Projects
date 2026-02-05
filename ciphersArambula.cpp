#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "include/caesar_dec.h"
#include "include/caesar_enc.h"
#include "include/subst_dec.h"
#include "include/subst_enc.h"
#include "utils.h"

using namespace std;

// Initialize random number generator in .cpp file for ODR reasons
std::mt19937 Random::rng;

const string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Function declarations go at the top of the file so we can call them
// anywhere in our program, such as in main or in other functions.
// Most other function declarations are in the included header
// files.

// When you add a new helper function, make sure to declare it up here!

/**
 * Print instructions for using the program.
 */
void printMenu();

int main() {
  Random::seed(time(NULL));
  string command;

  cout << "Welcome to Ciphers!" << endl;
  cout << "-------------------" << endl;
  cout << endl;

  ifstream fin("dictionary.txt");
  string word;
  vector<string> dictionary;
  while (fin >> word) {
    dictionary.push_back(word);
  }
  fin.close();

  vector<string> quadgrams;
  vector<int> counts;
  ifstream finn("english_quadgrams.txt");
  string line;
  while (finn >> line) {
    int pos = line.find(",");
    quadgrams.push_back(line.substr(0, pos));
    string num = line.substr(pos + 1, line.size());
    counts.push_back(stoi(num));
  }

  QuadgramScorer scorer = QuadgramScorer(quadgrams, counts);

  do {
    printMenu();
    cout << endl << "Enter a command (case does not matter): ";
    // Use getline for all user input to avoid needing to handle
    // input buffer issues relating to using both >> and getline
    getline(cin, command);
    cout << endl;

    if (command == "C" || command == "c") {
      caesarEncryptCommand();
    }

    if (command == "D" || command == "d") {
      caesarDecryptCommand(dictionary);
    }

    if (command == "A" || command == "a") {
      applyRandSubstCipherCommand();
    }

    if (command == "E" || command == "e") {
      computeEnglishnessCommand(scorer);
    }

    if (command == "S" || command == "s") {
      decryptSubstCipherCommand(scorer);
    }

    if (command == "F" || command == "f") {
      /// File
    }

    if (command == "R" || command == "r") {
      string seed_str;
      cout << "Enter a non-negative integer to seed the random number "
              "generator: ";
      getline(cin, seed_str);
      Random::seed(stoi(seed_str));
    }

    cout << endl;

  } while (!(command == "x" || command == "X") && !cin.eof());

  return 0;
}

void printMenu() {
  cout << "Ciphers Menu" << endl;
  cout << "------------" << endl;
  cout << "C - Encrypt with Caesar Cipher" << endl;
  cout << "D - Decrypt Caesar Cipher" << endl;
  cout << "E - Compute English-ness Score" << endl;
  cout << "A - Apply Random Substitution Cipher" << endl;
  cout << "S - Decrypt Substitution Cipher from Console" << endl;
  cout << "F - Decrypt Substitution Cipher from File" << endl;
  cout << "R - Set Random Seed for Testing" << endl;
  cout << "X - Exit Program" << endl;
}

// "#pragma region" and "#pragma endregion" group related functions in this file
// to tell VSCode that these are "foldable". You might have noticed the little
// down arrow next to functions or loops, and that you can click it to collapse
// those bodies. This lets us do the same thing for arbitrary chunks!
#pragma region CaesarEnc

char rot(char c, int amount) {
  int i = ALPHABET.find(c);
  if ((i + amount) > 25) {
    i = (i + amount) - 26;
  } else {
    i += amount;
  }
  char test = ALPHABET[i];
  return test;
}

string rot(const string& line, int amount) {
  string rotLine;
  for (char character : line) {
    if (isalpha(character)) {
      char test = rot(toupper(character), amount);
      rotLine += test;
    } else if (isspace(character)) {
      rotLine += " ";
    }
  }
  return rotLine;
}

void caesarEncryptCommand() {
  string line;
  string amountInput;
  cout << "Enter the text to encrypt:";
  getline(cin, line);
  cout << "Enter the number of characters to rotate by:";
  getline(cin, amountInput);
  int amount = stoi(amountInput);

  string rotated = rot(line, amount);
  cout << rotated << endl;
}

#pragma endregion CaesarEnc

#pragma region CaesarDec

void rot(vector<string>& strings, int amount) {
  int count = 0;
  for (string word : strings) {
    for (int i = 0; i < word.size(); i++) {
      char letter = word.at(i);
      word.at(i) = rot(letter, amount);
    }
    strings.at(count) = word;
    count++;
  }
}

string clean(const string& s) {
  string cleaned = "";
  for (int i = 0; i < s.size(); i++) {
    char character = s.at(i);
    if (isalpha(character)) {
      int index = ALPHABET.find(toupper(character));
      cleaned += ALPHABET[index];
    }
  }
  return cleaned;
}

vector<string> splitBySpaces(const string& s) {
  vector<string> wordList;
  stringstream ss(s);
  string word;
  while (ss >> word) {
    wordList.push_back(word);
  }
  return wordList;
}

string joinWithSpaces(const vector<string>& words) {
  string sentence = "";
  for (int i = 0; i < words.size(); i++) {
    if (i == words.size() - 1) {
      sentence += words[i];
    } else {
      sentence += words[i];
      sentence += " ";
    }
  }
  return sentence;
}

int numWordsIn(const vector<string>& words, const vector<string>& dict) {
  int count = 0;
  for (string wordVect : words) {
    for (string wordDict : dict) {
      if (wordVect == wordDict) {
        count++;
      }
    }
  }
  return count;
}

void caesarDecryptCommand(const vector<string>& dict) {
  string decrypt;
  cout << "Enter the text to Caesar decrypt:";
  getline(cin, decrypt);

  vector<string> vectDec = splitBySpaces(decrypt);
  for (int i = 0; i < vectDec.size(); i++) {
    vectDec[i] = clean(vectDec[i]);
  }

  int amountOutput = 0;
  vector<string> rotWords;

  for (int i = 0; i < 26; i++) {
    int correct = numWordsIn(vectDec, dict);

    if (correct > (vectDec.size() / 2)) {
      rotWords.push_back(joinWithSpaces(vectDec));
      amountOutput++;
    }
    rot(vectDec, 1);
  }
  if (amountOutput == 0) {
    cout << "No good decryptions found";
  } else {
    for (int i = 0; i < rotWords.size(); i++) {
      if (!(i == rotWords.size() - 1) || !(i == 0)) {
        cout << endl;
      }
      cout << rotWords.at(i);
    }
  }
}

#pragma endregion CaesarDec

#pragma region SubstEnc

string applySubstCipher(const vector<char>& cipher, const string& s) {
  string newString;
  for (int i = 0; i < s.size(); i++) {
    if (isspace(s[i])) {
      newString += " ";
    } else if (!isalpha(s[i])) {
      newString += s[i];
    } else {
      char letter = toupper(s[i]);
      int index = ALPHABET.find(letter);
      newString += cipher[index];
    }
  }
  return newString;
}

void applyRandSubstCipherCommand() {
  vector<char> cipher = genRandomSubstCipher();
  string s;
  getline(cin, s);
  string newString = applySubstCipher(cipher, s);
  cout << newString;
}

#pragma endregion SubstEnc

#pragma region SubstDec

double scoreString(const QuadgramScorer& scorer, const string& s) {
  double score = 0;

  vector<string> quads;

  int i = 0;
  while ((s.substr(i, 4)).size() == 4) {
    quads.push_back(s.substr(i, 4));
    i++;
  }
  for (int j = 0; j < quads.size(); j++) {
    score += scorer.getScore(quads[j]);
  }

  return score;
}

void computeEnglishnessCommand(const QuadgramScorer& scorer) {
  string input;
  cout << "Enter a string for englishness scoring:";
  getline(cin, input);
  double totalScore = 0;
  input = clean(input);
  totalScore = scoreString(scorer, input);
  cout << totalScore;
}

vector<char> hillClimb(const QuadgramScorer& scorer, const string& ciphertext) {
  double bestScore = 0;
  int counter = 0;
  vector<char> substitution = genRandomSubstCipher();
  vector<char> bestsub = substitution;
  string text = ciphertext;
  bestScore = scoreString(scorer, applySubstCipher(substitution, text));

  while (counter < 1000) {
    if (bestScore < scoreString(scorer, applySubstCipher(substitution, text))) {
      bestScore = scoreString(scorer, applySubstCipher(substitution, text));
      bestsub = substitution;
      counter = 0;
    } else {
      int index1 = Random::randInt(25);
      int index2 = Random::randInt(25);
      while (index1 == index2) {
        index2 = Random::randInt(25);
      }
      char temp = substitution[index1];
      substitution[index1] = substitution[index2];
      substitution[index2] = temp;
      counter++;
    }
    text = ciphertext;
  }
  return substitution;
}

vector<char> decryptSubstCipher(const QuadgramScorer& scorer,
                                const string& ciphertext) {
  double bestScore = 0;
  vector<char> bestsub;
  vector<vector<char> > listSubs;
  string cleaned = clean(ciphertext);
  for (int i = 0; i < 25; i++) {
    listSubs.push_back(hillClimb(scorer, cleaned));
  }
  bestScore = scoreString(scorer, applySubstCipher(listSubs[0], ciphertext));
  bestsub = listSubs[0];
  for (int p = 0; p < listSubs.size(); p++) {
    double tempScore =
        scoreString(scorer, applySubstCipher(listSubs[p], ciphertext));
    if (bestScore < tempScore) {
      bestScore = tempScore;
      bestsub = listSubs[p];
    }
  }
  return bestsub;
}

void decryptSubstCipherCommand(const QuadgramScorer& scorer) {
  string s;
  getline(cin, s);
  string cleanS = clean(s);
  vector<char> bestsub = decryptSubstCipher(scorer, cleanS);
  cleanS = applySubstCipher(bestsub, cleanS);
  int index = 0;
  string theString = "";
  for (int i = 0; i < s.size(); i++) {
    if (isspace(s[i])) {
      theString += " ";
    } else if (!isalpha(s[i])) {
      theString = +s[i];
    } else if (isalpha(s[i])) {
      theString += cleanS[index];
      index++;
    }
  }
  cout << theString;
}

#pragma endregion SubstDec
