

#include "config.h"
#include "TextBreakIterator.h"

#include "PlatformString.h"
#include <wtf/StdLibExtras.h>
#include <wtf/unicode/Unicode.h>

using namespace WTF::Unicode;

namespace WebCore {

// Hack, not entirely correct
static inline bool isCharStop(UChar c)
{
    CharCategory charCategory = category(c);
    return charCategory != Mark_NonSpacing && (charCategory != Other_Surrogate || (c < 0xd800 || c >= 0xdc00));
}

static inline bool isLineStop(UChar c)
{
    return category(c) != Separator_Line;
}

static inline bool isSentenceStop(UChar c)
{
    return isPunct(c);
}

class TextBreakIterator {
public:
    void reset(const UChar* str, int len)
    {
        string = str;
        length = len;
        currentPos = 0;
    }
    virtual int first() = 0;
    virtual int next() = 0;
    virtual int previous() = 0;
    int following(int position)
    {
        currentPos = position;
        return next();
    }
    int preceding(int position)
    {
        currentPos = position;
        return previous();
    }

    int currentPos;
    const UChar* string;
    int length;
};

struct WordBreakIterator: TextBreakIterator {
    virtual int first();
    virtual int next();
    virtual int previous();
};

struct CharBreakIterator: TextBreakIterator {
    virtual int first();
    virtual int next();
    virtual int previous();
};

struct LineBreakIterator: TextBreakIterator {
    virtual int first();
    virtual int next();
    virtual int previous();
};

struct SentenceBreakIterator : TextBreakIterator {
    virtual int first();
    virtual int next();
    virtual int previous();
};

int WordBreakIterator::first()
{
    currentPos = 0;
    return currentPos;
}

int WordBreakIterator::next()
{
    if (currentPos == length) {
        currentPos = -1;
        return currentPos;
    }
    bool haveSpace = false;
    while (currentPos < length) {
        if (haveSpace && !isSpace(string[currentPos]))
            break;
        if (isSpace(string[currentPos]))
            haveSpace = true;
        ++currentPos;
    }
    return currentPos;
}

int WordBreakIterator::previous()
{
    if (!currentPos) {
        currentPos = -1;
        return currentPos;
    }
    bool haveSpace = false;
    while (currentPos > 0) {
        if (haveSpace && !isSpace(string[currentPos]))
            break;
        if (isSpace(string[currentPos]))
            haveSpace = true;
        --currentPos;
    }
    return currentPos;
}

int CharBreakIterator::first()
{
    currentPos = 0;
    return currentPos;
}

int CharBreakIterator::next()
{
    if (currentPos >= length)
        return -1;
    ++currentPos;
    while (currentPos < length && !isCharStop(string[currentPos]))
        ++currentPos;
    return currentPos;
}

int CharBreakIterator::previous()
{
    if (currentPos <= 0)
        return -1;
    if (currentPos > length)
        currentPos = length;
    --currentPos;
    while (currentPos > 0 && !isCharStop(string[currentPos]))
        --currentPos;
    return currentPos;
}

int LineBreakIterator::first()
{
    currentPos = 0;
    return currentPos;
}

int LineBreakIterator::next()
{
    if (currentPos == length) {
        currentPos = -1;
        return currentPos;
    }
    bool haveSpace = false;
    while (currentPos < length) {
        if (haveSpace && !isLineStop(string[currentPos]))
            break;
        if (isLineStop(string[currentPos]))
            haveSpace = true;
        ++currentPos;
    }
    return currentPos;
}

int LineBreakIterator::previous()
{
    if (!currentPos) {
        currentPos = -1;
        return currentPos;
    }
    bool haveSpace = false;
    while (currentPos > 0) {
        if (haveSpace && !isLineStop(string[currentPos]))
            break;
        if (isLineStop(string[currentPos]))
            haveSpace = true;
        --currentPos;
    }
    return currentPos;
}

int SentenceBreakIterator::first()
{
    currentPos = 0;
    return currentPos;
}

int SentenceBreakIterator::next()
{
    if (currentPos == length) {
        currentPos = -1;
        return currentPos;
    }
    bool haveSpace = false;
    while (currentPos < length) {
        if (haveSpace && !isSentenceStop(string[currentPos]))
            break;
        if (isSentenceStop(string[currentPos]))
            haveSpace = true;
        ++currentPos;
    }
    return currentPos;
}

int SentenceBreakIterator::previous()
{
    if (!currentPos) {
        currentPos = -1;
        return currentPos;
    }
    bool haveSpace = false;
    while (currentPos > 0) {
        if (haveSpace && !isSentenceStop(string[currentPos]))
            break;
        if (isSentenceStop(string[currentPos]))
            haveSpace = true;
        --currentPos;
    }
    return currentPos;
}

TextBreakIterator* wordBreakIterator(const UChar* string, int length)
{
    DEFINE_STATIC_LOCAL(WordBreakIterator, iterator, ());
    iterator.reset(string, length);
    return &iterator;
}

TextBreakIterator* characterBreakIterator(const UChar* string, int length)
{
    DEFINE_STATIC_LOCAL(CharBreakIterator, iterator, ());
    iterator.reset(string, length);
    return &iterator;
}

TextBreakIterator* lineBreakIterator(const UChar* string, int length)
{
    DEFINE_STATIC_LOCAL(LineBreakIterator , iterator, ());
    iterator.reset(string, length);
    return &iterator;
}

TextBreakIterator* sentenceBreakIterator(const UChar* string, int length)
{
    DEFINE_STATIC_LOCAL(SentenceBreakIterator, iterator, ());
    iterator.reset(string, length);
    return &iterator;
}

int textBreakFirst(TextBreakIterator* breakIterator)
{
    return breakIterator->first();
}

int textBreakNext(TextBreakIterator* breakIterator)
{
    return breakIterator->next();
}

int textBreakPreceding(TextBreakIterator* breakIterator, int position)
{
    return breakIterator->preceding(position);
}

int textBreakFollowing(TextBreakIterator* breakIterator, int position)
{
    return breakIterator->following(position);
}

int textBreakCurrent(TextBreakIterator* breakIterator)
{
    return breakIterator->currentPos;
}

bool isTextBreak(TextBreakIterator*, int)
{
    return true;
}

TextBreakIterator* cursorMovementIterator(const UChar* string, int length)
{
    return characterBreakIterator(string, length);
}

} // namespace WebCore
