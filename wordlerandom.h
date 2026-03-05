#ifndef WORDLERANDOM_H
#define WORDLERANDOM_H

#include <chrono>
#include <random> //get(min, max) to execute the function in random
#include <string>
#include <vector>
#include <fstream>

namespace Random
{
	inline std::mt19937 generate()
	{
		std::random_device rd{};
		std::seed_seq ss{
			static_cast<std::seed_seq::result_type>(std::chrono::steady_clock::now().time_since_epoch().count()),
				rd(), rd(), rd(), rd(), rd(), rd(), rd() };

		return std::mt19937{ ss };
	}

	inline std::mt19937 mt{ generate() }; 

	inline int get(int min, int max)
	{
		return std::uniform_int_distribution{min, max}(mt);
	}


	template <typename T>
	T get(T min, T max)
	{
		return std::uniform_int_distribution<T>{min, max}(mt);
	}

	template <typename R, typename S, typename T>
	R get(S min, T max)
	{
		return get<R>(static_cast<R>(min), static_cast<R>(max));
	}
}

namespace WordleRandom
{
    inline const std::string path1{"dictionary.txt"};
	inline const std::string path2{"secretwords.txt"};

	inline std::vector<std::string> dictionary{};
	inline std::vector<std::string> secretWords{};

    inline bool loadWords()
    {
		std::string word{};

		std::ifstream dictionaryFile{path1};
		if (!dictionaryFile)
			return false;

		while (dictionaryFile >> word)
			if (word.size() == 5)
				dictionary.push_back(word);

		std::ifstream secretWordsFile{path2};
		if (!secretWordsFile)
			return false;

		while (secretWordsFile >> word)
			if (word.size() == 5)
				secretWords.push_back(word);
		
		return !dictionary.empty() && !secretWords.empty();
	}
}

#endif