#include "stdafx.h"
#include <locale>
#include <codecvt>

wstring toLower(const wstring& a)
{
	wstring out = wstring(a);
	std::transform(out.begin(), out.end(), out.begin(), ::tolower);
	return out;
}

wstring trimString(const wstring& a)
{
	wstring b;
	size_t start = a.find_first_not_of(L" \t\n\r");
	size_t end = a.find_last_not_of(L" \t\n\r");
	if( start == wstring::npos ) start = 0;
	if( end == wstring::npos ) end = a.size() - 1;
	b = a.substr(start,(end-start)+1);
	return b;
}

wstring replaceAll(const wstring& in, const wstring& replace, const wstring& with)
{
	wstring out = in;
	size_t pos = 0;
	while( ( pos = out.find(replace, pos) ) != wstring::npos )
	{
		out.replace( pos, replace.length(), with );
		pos++;
	}
	return out;
}

bool equalsIgnoreCase(const wstring& a, const wstring& b)
{
	bool out;
	wstring c = toLower(a);
	wstring d = toLower(b);
	out = c.compare(d) == 0;
	return out;
}

wstring convStringToWstring(const string& converting)
{
	if (converting.empty()) return wstring();

#ifdef _WIN32
	// windows - UTF-16
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
#else
	// unix - UTF-32
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
#endif
	try
	{
		return converter.from_bytes(converting);
	}
	catch(...)
	{
		// fallbakc - simple widening
		wstring converted(converting.length(), L' ');
		copy(converting.begin(), converting.end(), converted.begin());
		return converted;
	}
}

// Convert for filename wstrings to a straight character pointer for Xbox APIs. The returned string is only valid until
// this function is called again, and it isn't thread-safe etc. as I'm just storing the returned name in a local static
// to save having to clear it up everywhere this is used.
const char *wstringtofilename(const wstring& name)
{
    static thread_local char buf[4][8192];
    static thread_local int bufIndex = 0;

    char *out = buf[bufIndex];
    bufIndex = (bufIndex + 1) % 4;

    size_t len = name.length();
    if (len >= sizeof(buf[0]))
    {
#ifndef _CONTENT_PACKAGE
        app.DebugPrintf("wstringtofilename: path too long, truncating from %zu to %zu\n", len, sizeof(buf[0]) - 1);
#endif
        len = sizeof(buf[0]) - 1;
    }

    for (size_t i = 0; i < len; ++i)
    {
        wchar_t c = name[i];
#if defined __PS3__ || defined __ORBIS__
        if (c == L'\\') c = L'/';
#else
        if (c == L'/') c = L'\\';
#endif
        if (c >= 128)
        {
#ifndef _CONTENT_PACKAGE
            app.DebugPrintf("Non-ASCII character in filename: %lc\n", c);
#endif
            c = L'?';
        }
        out[i] = static_cast<char>(c);
    }
    out[len] = '\0';

    return out;
}

const char *wstringtochararray(const wstring& name)
{
    static thread_local char buf[4][8192];
    static thread_local int bufIndex = 0;

    char *out = buf[bufIndex];
    bufIndex = (bufIndex + 1) % 4;

    size_t len = name.length();
    if (len >= sizeof(buf[0]))
    {
#ifndef _CONTENT_PACKAGE
        app.DebugPrintf("wstringtochararray: string too long, truncating from %zu to %zu\n", len, sizeof(buf[0]) - 1);
#endif
        len = sizeof(buf[0]) - 1;
    }

    for (size_t i = 0; i < len; ++i)
    {
        wchar_t c = name[i];
        if (c >= 128)
        {
#ifndef _CONTENT_PACKAGE
            app.DebugPrintf("Non-ASCII character in string conversion: %lc\n", c);
#endif
            c = L'?';
        }
        out[i] = static_cast<char>(c);
    }
    out[len] = '\0';

    return out;

}

wstring filenametowstring(const char *name)
{
	return convStringToWstring(name);
}

std::vector<std::wstring> &stringSplit(const std::wstring &s, wchar_t delim, std::vector<std::wstring> &elems)
{
    std::wstringstream ss(s);
    std::wstring item;
    while(std::getline(ss, item, delim))
	{
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::wstring> stringSplit(const std::wstring &s, wchar_t delim)
{
    std::vector<std::wstring> elems;
    return stringSplit(s, delim, elems);
}

bool BothAreSpaces(wchar_t lhs, wchar_t rhs) { return (lhs == rhs) && (lhs == L' '); }

void stripWhitespaceForHtml(wstring &string, bool bRemoveNewline)
{
	// Strip newline chars
	if(bRemoveNewline)
	{	
		string.erase(std::remove(string.begin(), string.end(), '\n'), string.end());
		string.erase(std::remove(string.begin(), string.end(), '\r'), string.end());
	}

	string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());

	// Strip duplicate spaces
	string.erase(std::unique(string.begin(), string.end(), BothAreSpaces), string.end()); 

	string = trimString(string);
}

wstring escapeXML(const wstring &in)
{
	wstring out = in;
	out = replaceAll(out, L"&", L"&amp;");
	//out = replaceAll(out, L"\"", L"&quot;");
	//out = replaceAll(out, L"'", L"&apos;");
	out = replaceAll(out, L"<", L"&lt;");
	out = replaceAll(out, L">", L"&gt;");
	return out;
}

wstring parseXMLSpecials(const wstring &in)
{
	wstring out = in;
	out = replaceAll(out, L"&amp;", L"&");
	//out = replaceAll(out, L"\"", L"&quot;");
	//out = replaceAll(out, L"'", L"&apos;");
	out = replaceAll(out, L"&lt;", L"<");
	out = replaceAll(out, L"&gt;", L">");
	return out;
}