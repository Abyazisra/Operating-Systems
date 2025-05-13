#include <iostream>
#include <map>
#include <thread>
#include <mutex>
#include <sstream>
#include <string>

using namespace std;

mutex mtx;

/*
======================
Function to split a string into words and return as an array
======================
*/
int splitString(const string& str, string words[], int maxSize) {
    istringstream stream(str);
    string word;
    int index = 0;
    while (stream >> word && index < maxSize) {
        words[index++] = word;
    }
    return index; // Returns the number of words
}

/*
======================
Map Phase
======================
*/
void mapFunction(const string* chunk, int chunkSize, map<string, int>& intermediate) {
    for (int i = 0; i < chunkSize; ++i) {
        lock_guard<mutex> lock(mtx);
        intermediate[chunk[i]]++; // Emit (key, 1)
    }
}

/*
======================
Shuffle Phase
======================
*/
void shufflePhase(const map<string, int>& intermediate1, 
                  const map<string, int>& intermediate2, 
                  map<string, int>& shuffled) {
    lock_guard<mutex> lock(mtx);
    for (const auto& pair : intermediate1) {
        shuffled[pair.first] += pair.second;
    }
    for (const auto& pair : intermediate2) {
        shuffled[pair.first] += pair.second;
    }
}

/*
======================
Reduce Phase
======================
*/
void reduceFunction(const map<string, int>& shuffled, map<string, int>& finalOutput) {
    for (const auto& pair : shuffled) {
        finalOutput[pair.first] = pair.second; // Sum values
    }
}

int main() {
    // Input Phase: Taking input from the user
    string input;
    cout << "Enter a string of words: ";
    getline(cin, input);

    // Splitting input into words using arrays
    const int maxWords = 100; // Assuming a maximum of 100 words for simplicity
    string words[maxWords];
    int wordCount = splitString(input, words, maxWords);

    /*
    ======================
    Map Phase
    ======================
    */
    int chunkSize = wordCount / 2;

    // Dynamic allocation of chunks
    string* chunk1 = new string[chunkSize];
    string* chunk2 = new string[wordCount - chunkSize];

    // Divide the data into two chunks
    for (int i = 0; i < chunkSize; ++i) {
        chunk1[i] = words[i];
    }
    for (int i = chunkSize; i < wordCount; ++i) {
        chunk2[i - chunkSize] = words[i];
    }

    // Intermediate data structures
    map<string, int> intermediate1, intermediate2;

    // Creating threads for map phase
    thread t1(mapFunction, chunk1, chunkSize, ref(intermediate1));
    thread t2(mapFunction, chunk2, wordCount - chunkSize, ref(intermediate2));

    t1.join();
    t2.join();

    /*
    ======================
    Shuffle Phase
    ======================
    */
    map<string, int> shuffled;
    shufflePhase(intermediate1, intermediate2, shuffled);

    /*
    ======================
    Reduce Phase
    ======================
    */
    map<string, int> finalOutput;
    reduceFunction(shuffled, finalOutput);

    // Output the final results
    cout << "Final Output:" << endl;
    for (const auto& pair : finalOutput) {
        cout << "(" << pair.first << ", " << pair.second << ")" << endl;
    }

    // Clean up dynamically allocated memory
    delete[] chunk1;
    delete[] chunk2;

    return 0;
}

