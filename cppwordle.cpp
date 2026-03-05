#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cctype>
#include <limits>
#include <fstream>
#include "wordlerandom.h"

constexpr int wordLength{5};
constexpr int guessAttempts{6};

namespace Color
{
    constexpr std::string_view GREEN{"\033[32m"};
    constexpr std::string_view YELLOW{"\033[33m"};
    constexpr std::string_view GRAY{"\033[90m"};
    constexpr std::string_view WHITE{"\033[0m"};
}

void loadWordList()
{
    if (!WordleRandom::loadWords())
        std::exit(EXIT_FAILURE);
}


std::string getSecretWord()
{
    using namespace WordleRandom;

    return secretWords[Random::get(0, secretWords.size() - 1)];
}

void getUpperOrLower(char choice, std::string& text) //u is upper, l is lower
{
    switch (choice)
    {
    case 'u': 
    case 'U':
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {return static_cast<unsigned char>(std::toupper(c));});
        break;
    case 'l': 
    case 'L':
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {return static_cast<unsigned char>(std::tolower(c));});
        break;
    }
}

constexpr std::size_t charToIndex(char c){return static_cast<std::size_t>(c - 'a');}

class Session
{
private:
    std::string m_word{getSecretWord()};
    std::vector<std::string> m_userGuesses{};
    std::array<int, 26> m_letterStates{};
    bool m_wonGame{false};
public:
    std::string_view getWord() const {return m_word;}
    void addUserGuess(const std::string& userGuess)
    {
        m_userGuesses.push_back(userGuess);
    }
    int getUserAttempts() const {return m_userGuesses.size();}

    std::string_view getLatestGuess() const {return m_userGuesses[std::size(m_userGuesses) - 1];}
    void correctlyGuessed(){m_wonGame = true;}
    bool wonGame(){return m_wonGame;}

    void addLatestGuessAmount(std::array<int, guessAttempts + 1>& wordGuessAttempts)
    {
        if (m_wonGame)
            ++wordGuessAttempts[m_userGuesses.size() - 1];
        else
            ++wordGuessAttempts[guessAttempts];

        m_userGuesses.clear();
    }

    void changeLetterState(char letter, int type) //0 is not guessed, 1 is gray, 2 is yellow, 3 is green
    {
        const std::size_t idx = charToIndex(letter);

        if (type > m_letterStates[idx])
            m_letterStates[idx] = type;
    }
    const std::array<int, 26>& getLetterStates() const {return m_letterStates;}
};

void printLetterStates(const Session& info)
{   
    int idx{0};
    for (const auto& i : info.getLetterStates())
    {
        switch (i)
        {
        case 0: 
            std::cout << static_cast<char>(idx + 97);
            break;
        case 1: 
            std::cout << Color::GRAY << static_cast<char>(idx + 97) << Color::WHITE;
            break;
        case 2: 
            std::cout << Color::YELLOW << static_cast<char>(idx + 97) << Color::WHITE;
            break;
        case 3: 
            std::cout << Color::GREEN << static_cast<char>(idx + 97) << Color::WHITE;
            break;
        }
        if (idx == 12)
            std::cout << '\n';
        ++idx;
    }
    std::cout << '\n';
}

void getUserGuess(Session& info)
{
    std::string userGuess{};
    while (true)
    {
        bool invalidCharacter{false};
        bool invalidWord{true};
        std::cout << "Enter your word: ";
        
        std::getline(std::cin, userGuess);

        getUpperOrLower('l', userGuess);
        if (userGuess == "view")
        {
            printLetterStates(info);
            continue;
        }

        if (userGuess.size() != wordLength)
        {
            std::cout << "Enter a " << wordLength << " letter word!\n";
            continue;
        }
        
        for (const auto& c : userGuess)
        {
            invalidCharacter = !(std::isalpha(c));
            if (invalidCharacter)
            {
                std::cout << "Make sure the word only has letters!\n";
                break;
            }
        }

        for (const auto& word : WordleRandom::dictionary)
        {
            if (word == userGuess)
            {
                invalidWord = false;
                break;
            }
        }
        if (invalidWord)
        {
            std::cout << "Enter a viable word!\n";
            continue;
        }

        if (!invalidCharacter && !invalidWord)
        {
            info.addUserGuess(userGuess);
            return;
        }
    }
}

bool isCharInString(const Session& info, char c)
{
    for (std::size_t i{0}; i < info.getLatestGuess().size(); ++i)
    {
        if ((info.getWord())[i] == c)
            return true;
    }

    return false;
}

void printWordState(Session& info)
{
    if (info.getLatestGuess() == info.getWord())
    {
        info.correctlyGuessed();
        std::cout << Color::GREEN << info.getLatestGuess() << Color::WHITE << '\n';
        return;
    }

    std::array<int, 26> letterCount{}; //stores the contained letters for each index for secret word
    std::array<int, wordLength> wordStatus{}; //3 is green, 2 is yellow, 1 is gray
    for (const auto& c : info.getWord())
        ++letterCount[charToIndex(c)]; 

    for (std::size_t i{0}; i < wordLength; ++i)
    {
        if (info.getLatestGuess()[i] == info.getWord()[i])
        {
            wordStatus[i] = 1;
            --letterCount[charToIndex(std::tolower(info.getLatestGuess()[i]))];
        }
    }
    for (std::size_t i{0}; i < wordLength; ++i)
    {
        const std::size_t idx{charToIndex(std::tolower(info.getLatestGuess()[i]))};
        if (wordStatus[i] == 1) //checks green
        {
            info.changeLetterState(info.getLatestGuess()[i], 3);
            continue;
        }
        if (letterCount[idx] > 0) //checks yellow
        {
            info.changeLetterState(info.getLatestGuess()[i], 2);
            wordStatus[i] = 2;
            --letterCount[idx];
        }
        else
        {
            info.changeLetterState(info.getLatestGuess()[i], 1);
            wordStatus[i] = 0; //gray
        }
    }

    for (std::size_t i{0}; i < wordLength; ++i)
    {
        switch (wordStatus[i])
        {
        case 1: 
            std::cout << Color::GREEN << (info.getLatestGuess())[i] << Color::WHITE;
            break;
        case 2: 
            std::cout << Color::YELLOW << (info.getLatestGuess())[i] << Color::WHITE;
            break;
        default: std::cout << Color::GRAY << (info.getLatestGuess())[i] << Color::WHITE;
        }
    }

    std::cout << '\n';
}

std::string getPlural(int amount, const std::string& word)
{
    if (amount == 1)
        return word;
    return word + "s";
}

void printGuessStats(const std::array<int, guessAttempts + 1>& wordGuessAttempts)
{
    for (std::size_t i{0}; i < guessAttempts; ++i)
    {
        if (wordGuessAttempts[i] > 0)
            std::cout << "You guessed " << wordGuessAttempts[i] << ' ' << getPlural(wordGuessAttempts[i], "word") << " in " << i + 1 << ' ' << getPlural(i + 1, "attempt") << '\n';
    }
    if (wordGuessAttempts[guessAttempts] > 0)
        std::cout << "You couldn't guess " << wordGuessAttempts[guessAttempts] << ' ' << getPlural(wordGuessAttempts[guessAttempts], "word") << '\n';
}

template <typename T>
T getCheckedInput(std::string_view statement)
{
    std::cout << statement << '\n';

    T input{};
        
    while (!(std::cin >> input))
    {
        std::cout << "Invalid Input!\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    
    return input;
}

int main()
{
    if (!WordleRandom::loadWords())
        std::exit(EXIT_FAILURE);

    std::cout << "Hello! This is WORDLE, a word game where you try to guess the word!\n";
    std::cout << Color::GREEN << "Green" << Color::WHITE << " means that you have the right letter put in the right spot\n";
    std::cout << Color::YELLOW << "Yellow" << Color::WHITE << " means that you have the right letter in the wrong spot\n";
    std::cout << Color::GRAY << "Gray" << Color::WHITE << " means that you don't have the right letter\n";
    std::cout << "You can always type \"view\" to look at the letter status (green, yellow, gray, or unguessed)\nHave fun!\n";

    std::array<int, guessAttempts + 1> wordGuessAttempts{};
    bool keepPlaying{true};

    do
    {
        Session info{};

        for (int i{0}; i < guessAttempts; ++i)
        {
            getUserGuess(info);
            
            printWordState(info);
            if (info.wonGame())
                break;
        }

        if (info.wonGame())
            std::cout << "Nice Job! You guessed the word!\n";
        else
            std::cout << "You couldn't get the word this time... The word was " << info.getWord() << '\n';
        
        info.addLatestGuessAmount(wordGuessAttempts);

        bool exitLoop{false};
        while (!exitLoop)
        {
            char playAgain{getCheckedInput<char>("Would you like to play again (y / n), or see your stats (s)?")};
            
            switch (playAgain)
            {
            case 'y':
                keepPlaying = true;
                exitLoop = true;
                break;
            case 'n':
                keepPlaying = false;
                exitLoop = true;
                break;
            case 's':
                printGuessStats(wordGuessAttempts);
                break;
            default: std::cout << "Invalid Input!\n";
            }
        }

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } while (keepPlaying);

    std::cout << "Thank you for playing " << Color::GREEN << "WORDLE" << Color::WHITE << '\n';
    return 0;
}