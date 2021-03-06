#ifndef _MULTIPART_PARSER_H_
#define _MULTIPART_PARSER_H_

#include <sys/types.h>
#include <string>
#include <stdexcept>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <set>

// #define DEBUG

class MultipartParser {
public:
	typedef void (MultipartParser::*Callback)(const char *buffer, size_t start, size_t end, void *userData);
	
private:
	static const char CR     = 13;
	static const char LF     = 10;
	static const char SPACE  = 32;
	static const char HYPHEN = 45;
	static const char COLON  = 58;
	static const size_t UNMARKED = (size_t) -1;
	
	enum State {
		ERROR,
		START,
		START_BOUNDARY,
		HEADER_FIELD_START,
		HEADER_FIELD,
		HEADER_VALUE_START,
		HEADER_VALUE,
		HEADER_VALUE_ALMOST_DONE,
		HEADERS_ALMOST_DONE,
		PART_DATA_START,
		PART_DATA,
		PART_END,
		END
	};
	
	enum Flags {
		PART_BOUNDARY = 1,
		LAST_BOUNDARY = 2
	};
	
	std::string boundary;
	const char *boundaryData;
	size_t boundarySize;
	bool boundaryIndex[256];
	char *lookbehind;
	size_t lookbehindSize;
	State state;
	int flags;
	size_t index;
	size_t headerFieldMark;
	size_t headerValueMark;
	size_t partDataMark;
	const char *errorReason;
	
	void resetCallbacks() {
		onPartBegin   = NULL;
		onHeaderField = NULL;
		onHeaderValue = NULL;
		onHeaderEnd   = NULL;
		onHeadersEnd  = NULL;
		onPartData    = NULL;
		onPartEnd     = NULL;
		onEnd         = NULL;
		userData      = NULL;
	}
	
	void indexBoundary() {
		const char *current;
		const char *end = boundaryData + boundarySize;
		
		memset(boundaryIndex, 0, sizeof(boundaryIndex));
		
		for (current = boundaryData; current < end; current++) {
			boundaryIndex[(unsigned char) *current] = true;
		}
	}
	
	void callback(Callback cb, const char *buffer = NULL, size_t start = UNMARKED,
		size_t end = UNMARKED, bool allowEmpty = false)
	{
		if (start != UNMARKED && start == end && !allowEmpty) {
			return;
		}
		if (cb != NULL) {
			(this->*cb)(buffer, start, end, userData);
		}
	}
	
	void dataCallback(Callback cb, size_t &mark, const char *buffer, size_t i, size_t bufferLen,
		bool clear, bool allowEmpty = false)
	{
		if (mark == UNMARKED) {
			return;
		}
		
		if (!clear) {
			callback(cb, buffer, mark, bufferLen, allowEmpty);
			mark = 0;
		} else {
			callback(cb, buffer, mark, i, allowEmpty);
			mark = UNMARKED;
		}
	}
	
	char lower(char c) const {
		return c | 0x20;
	}
	
	inline bool isBoundaryChar(char c) const {
		return boundaryIndex[(unsigned char) c];
	}
	
	bool isHeaderFieldCharacter(char c) const {
		return (c >= 'a' && c <= 'z')
			|| (c >= 'A' && c <= 'Z')
			|| c == HYPHEN;
	}
	
	void setError(const char *message) {
		state = ERROR;
		errorReason = message;
	}
	
	void processPartData(size_t &prevIndex, size_t &index, const char *buffer,
		size_t len, size_t boundaryEnd, size_t &i, char c, State &state, int &flags)
	{
		prevIndex = index;
		
		if (index == 0) {
			// boyer-moore derived algorithm to safely skip non-boundary data
			while (i + boundarySize <= len) {
				if (isBoundaryChar(buffer[i + boundaryEnd])) {
					break;
				}
				
				i += boundarySize;
			}
			if (i == len) {
				return;
			}
			c = buffer[i];
		}
		
		if (index < boundarySize) {
			if (boundary[index] == c) {
				if (index == 0) {
					dataCallback(onPartData, partDataMark, buffer, i, len, true);
				}
				index++;
			} else {
				index = 0;
			}
		} else if (index == boundarySize) {
			index++;
			if (c == CR) {
				// CR = part boundary
				flags |= PART_BOUNDARY;
			} else if (c == HYPHEN) {
				// HYPHEN = end boundary
				flags |= LAST_BOUNDARY;
			} else {
				index = 0;
			}
		} else if (index - 1 == boundarySize) {
			if (flags & PART_BOUNDARY) {
				index = 0;
				if (c == LF) {
					// unset the PART_BOUNDARY flag
					flags &= ~PART_BOUNDARY;
					callback(onPartEnd);
					callback(onPartBegin);
					state = HEADER_FIELD_START;
					return;
				}
			} else if (flags & LAST_BOUNDARY) {
				if (c == HYPHEN) {
                    callback(onPartEnd);
                    callback(onEnd);
                    state = END;
				} else {
					index = 0;
				}
			} else {
				index = 0;
			}
		} else if (index - 2 == boundarySize) {
			if (c == CR) {
				index++;
			} else {
				index = 0;
			}
		} else if (index - boundarySize == 3) {
			index = 0;
			if (c == LF) {
				callback(onPartEnd);
				callback(onEnd);
				state = END;
				return;
			}
		}
		
		if (index > 0) {
			// when matching a possible boundary, keep a lookbehind reference
			// in case it turns out to be a false lead
			if (index - 1 >= lookbehindSize) {
				setError("Parser bug: index overflows lookbehind buffer. "
					"Please send bug report with input file attached.");
				throw std::out_of_range("index overflows lookbehind buffer");
			} else if ((index - 1) < 0) {
				setError("Parser bug: index underflows lookbehind buffer. "
					"Please send bug report with input file attached.");
				throw std::out_of_range("index underflows lookbehind buffer");
			}
			lookbehind[index - 1] = c;
		} else if (prevIndex > 0) {
			// if our boundary turned out to be rubbish, the captured lookbehind
			// belongs to partData
			callback(onPartData, lookbehind, 0, prevIndex);
			prevIndex = 0;
			partDataMark = i;
			
			// reconsider the current character even so it interrupted the sequence
			// it could be the beginning of a new sequence
			i--;
		}
	}
private:
	void onPartBeginCallback(const char *buffer, size_t start, size_t end, void *userData)
	{
		#ifdef DEBUG
		printf("onPartBegin\n");
		#endif
	}

	void onHeaderFieldCallback(const char *buffer, size_t start, size_t end, void *userData)
	{
		this->fieldData.push_back(std::string(buffer + start, end - start));
		#ifdef DEBUG
		printf("onHeaderField: (%s)\n", std::string(buffer + start, end - start).c_str());
		#endif
	}

	void onHeaderValueCallback(const char *buffer, size_t start, size_t end, void *userData)
	{
		this->valueData.push_back(std::string(buffer + start, end - start));
		#ifdef DEBUG
		printf("onHeaderValue: (%s)\n", std::string(buffer + start, end - start).c_str());
		#endif
	}

	void onPartDataCallback(const char *buffer, size_t start, size_t end, void *userData)
	{
		std::string data = std::string(buffer + start, end - start);
		if (data.compare("\r\n") != 0)
		{
			// ?????????"\r\n"
			this->bodyData.insert(data);
		}
		
		#ifdef DEBUG
		printf("onPartData: (%s)\n", std::string(buffer + start, end - start).c_str());
		#endif
	}

	void onPartEndCallback(const char *buffer, size_t start, size_t end, void *userData)
	{
		#ifdef DEBUG
		printf("onPartEnd\n");
		#endif
	}

	void onEndCallback(const char *buffer, size_t start, size_t end, void *userData)
	{
		#ifdef DEBUG
		printf("onEnd\n");
		#endif
	}

	void init()
	{
		this->onPartBegin = &MultipartParser::onPartBeginCallback;
		this->onHeaderField = &MultipartParser::onHeaderFieldCallback;
		this->onHeaderValue = &MultipartParser::onHeaderValueCallback;
		this->onPartData = &MultipartParser::onPartDataCallback;
		this->onPartEnd = &MultipartParser::onPartEndCallback;
		this->onEnd = &MultipartParser::onEndCallback;
	}
	
public:
	Callback onPartBegin;
	Callback onHeaderField;
	Callback onHeaderValue;
	Callback onHeaderEnd;
	Callback onHeadersEnd;
	Callback onPartData;
	Callback onPartEnd;
	Callback onEnd;
	void *userData;
	std::unordered_map<std::string, std::string> reflectData;
	std::set<std::string> bodyData;
	std::vector<std::string> fieldData;
	std::vector<std::string> valueData;
	
	MultipartParser() {
		lookbehind = NULL;
		resetCallbacks();
		reset();
		init();
	}
	
	MultipartParser(const std::string &boundary) {
		lookbehind = NULL;
		resetCallbacks();
		setBoundary(boundary);
	}
	
	~MultipartParser() {
		delete[] lookbehind;
	}
	
	void reset() {
		delete[] lookbehind;
		state = ERROR;
		boundary.clear();
		boundaryData = boundary.c_str();
		boundarySize = 0;
		lookbehind = NULL;
		lookbehindSize = 0;
		flags = 0;
		index = 0;
		headerFieldMark = UNMARKED;
		headerValueMark = UNMARKED;
		partDataMark    = UNMARKED;
		errorReason     = "Parser uninitialized.";
	}
	
	void setBoundary(const std::string &boundary) {
		reset();
		// this->boundary = "\r\n--" + boundary;
		// this->boundary = boundary;
		size_t pos = boundary.find_first_of('\r');
		this->boundary = "\r\n" + boundary.substr(0, pos);
		this->boundary = boundary.substr(0, pos);
		printf("this->boundary = %s\n", this->boundary.c_str());
		boundaryData = this->boundary.c_str();
		boundarySize = this->boundary.size();
		indexBoundary();
		lookbehind = new char[boundarySize + 8];
		lookbehindSize = boundarySize + 8;
		state = START;
		errorReason = "No error.";
	}
	
	size_t feed(const char *buffer, size_t len) {
		if (state == ERROR || len == 0) {
			return 0;
		}
		
		State state         = this->state;
		int flags           = this->flags;
		size_t prevIndex    = this->index;
		size_t index        = this->index;
		size_t boundaryEnd  = boundarySize - 1;
		size_t i;
		char c, cl;
		
		for (i = 0; i < len; i++) {
			c = buffer[i];
			
			switch (state) {
			case ERROR:
				return i;
			case START:
				index = 0;
				state = START_BOUNDARY;
			case START_BOUNDARY:
				if (index == boundarySize - 2) {
					if (c != CR) {
						setError("Malformed. Expected CR after boundary.");
						return i;
					}
					index++;
					break;
				} else if (index - 1 == boundarySize - 2) {
					if (c != LF) {
						setError("Malformed. Expected LF after boundary CR.");
						return i;
					}
					index = 0;
					callback(onPartBegin);
					state = HEADER_FIELD_START;
					break;
				}
				if (c != boundary[index + 2]) {
					setError("Malformed. Found different boundary data than the given one.");
					return i;
				}
				index++;
				break;
			case HEADER_FIELD_START:
				state = HEADER_FIELD;
				headerFieldMark = i;
				index = 0;
			case HEADER_FIELD:
				if (c == CR) {
					headerFieldMark = UNMARKED;
					state = HEADERS_ALMOST_DONE;
					break;
				}

				index++;
				if (c == HYPHEN) {
					break;
				}

				if (c == COLON) {
					if (index == 1) {
						// empty header field
						setError("Malformed first header name character.");
						return i;
					}
					dataCallback(onHeaderField, headerFieldMark, buffer, i, len, true);
					state = HEADER_VALUE_START;
					break;
				}

				cl = lower(c);
				if (cl < 'a' || cl > 'z') {
					setError("Malformed header name.");
					return i;
				}
				break;
			case HEADER_VALUE_START:
				if (c == SPACE) {
					break;
				}
				
				headerValueMark = i;
				state = HEADER_VALUE;
			case HEADER_VALUE:
				if (c == CR) {
					dataCallback(onHeaderValue, headerValueMark, buffer, i, len, true, true);
					callback(onHeaderEnd);
					state = HEADER_VALUE_ALMOST_DONE;
				}
				break;
			case HEADER_VALUE_ALMOST_DONE:
				if (c != LF) {
					setError("Malformed header value: LF expected after CR");
					return i;
				}
				
				state = HEADER_FIELD_START;
				break;
			case HEADERS_ALMOST_DONE:
				if (c != LF) {
					setError("Malformed header ending: LF expected after CR");
					return i;
				}
				
				callback(onHeadersEnd);
				state = PART_DATA_START;
				break;
			case PART_DATA_START:
				state = PART_DATA;
				partDataMark = i;
			case PART_DATA:
				processPartData(prevIndex, index, buffer, len, boundaryEnd, i, c, state, flags);
				break;
			default:
				return i;
			}
		}
		
		dataCallback(onHeaderField, headerFieldMark, buffer, i, len, false);
		dataCallback(onHeaderValue, headerValueMark, buffer, i, len, false);
		dataCallback(onPartData, partDataMark, buffer, i, len, false);
		
		this->index = index;
		this->state = state;
		this->flags = flags;
		
		return len;
	}
	
	bool succeeded() const {
		return state == END;
	}
	
	bool hasError() const {
		return state == ERROR;
	}
	
	bool stopped() const {
		return state == ERROR || state == END;
	}
	
	const char *getErrorMessage() const {
		return errorReason;
	}
	// ?????????????????????
	void genReflectData()
	{
		for (size_t i = 0; i < this->fieldData.size(); i++)
		{
			this->reflectData.insert(std::make_pair(this->fieldData[i], this->valueData[i]));
		}
	}
};

#endif /* _MULTIPART_PARSER_H_ */
