#pragma once

#ifndef LZ_SPLIT_ITERATOR_HPP
#define LZ_SPLIT_ITERATOR_HPP

#include <string>

#include "LzTools.hpp"

#ifdef LZ_HAS_STRING_VIEW
  #include <string_view>
#endif


namespace lz { namespace internal {
template<class SubString, class String>
class SplitIterator {
	std::size_t _currentPos{}, _last{};
	const String* _string{nullptr};
	std::string _delimiter;

public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = SubString;
	using reference = SubString;
	using difference_type = std::ptrdiff_t;
	using pointer = FakePointerProxy<reference>;

	SplitIterator(const std::size_t startingPosition, const String& string, std::string delimiter) :
		_currentPos(startingPosition),
		_string(&string),
		_delimiter(std::move(delimiter)) {
		// Micro optimization, check if object is created from begin(), only then we want to search
		if (startingPosition == 0) {
			_last = _string->find(_delimiter, _currentPos);
		}
	}

	SplitIterator() = default;

	value_type operator*() const {
		if (_last != std::string::npos) {
			return SubString(&(*_string)[_currentPos], _last - _currentPos);
		}
		else {
			return SubString(&(*_string)[_currentPos]);
		}
	}

	pointer operator->() const {
		return FakePointerProxy<decltype(**this)>(**this);
	}

	friend bool operator!=(const SplitIterator& a, const SplitIterator& b) {
		return a._currentPos != b._currentPos;
	}

	friend bool operator==(const SplitIterator& a, const SplitIterator& b) {
		return !(a != b); // NOLINT
	}

	SplitIterator& operator++() {
		const std::size_t delimLen = _delimiter.length();
		const std::size_t stringLen = _string->length();
		if (_last == std::string::npos) {
			_currentPos = stringLen;
		}
		else if (_last == stringLen - delimLen) {
			// Check if ends with delimiter
			_last = std::string::npos;
			_currentPos = _string->length();
		}
		else {
			_currentPos = _last + delimLen;
			_last = _string->find(_delimiter, _currentPos);
		}
		return *this;
	}

	SplitIterator operator++(int) {
		SplitIterator tmp(*this);
		++*this;
		return tmp;
	}
};
}}

#endif