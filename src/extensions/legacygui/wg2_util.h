#ifndef WG2_UTIL_DOT_H
#define WG2_UTIL_DOT_H

#include <string>
#include <sstream>
#include <vector>
#include <wg2_geo.h>

#ifndef WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

class WgBlock;
class WgWidget;
class WgPanelHook;

namespace wg
{
	struct TextAttr;
	class Char;
}


//____ WgUtil _________________________________________________________________

namespace WgUtil
{
	float       ParametricBlendInOut(float t);


	bool		AdjustScaledArea(const WgBlock& block, WgRect& area);

	bool		MarkTestBlock( WgCoord ofs, const WgBlock& block, WgRect area, int opacityTreshold );

	WgCoord 	OrigoToOfs( WgOrigo origo, WgSize base );
	WgRect		OrigoToRect( WgOrigo origo, WgSize base, WgSize rect );

	int 		SizeFromPolicy( int defaultSize, int specifiedSize, WgSizePolicy policy );

	inline wg::Axis DirToOrient( WgDirection dir ) { return (dir == WgDirection::Up || dir == WgDirection::Down) ? wg::Axis::Y : wg::Axis::X; }

	template <class T> inline T Max (T a, T b) {return (a>b?a:b); }
	template <class T> inline T Min (T a, T b) {return (a<b?a:b); }

	WgMode		StateToMode(WgState state);
	WgState     ModeToState(WgMode mode);


	uint32_t lineWidth( void * pNode, const wg::TextAttr& attr, wg::State mode, const wg::Char * pString, int scale );

	template<typename T>
	inline std::string ToString(T value)
	{
		std::ostringstream os;
		os << value;
		return os.str();
	}

	// These two are needed to print the value as an integer instead of an ascii character
	template<> inline std::string ToString(Sint8 value) { return ToString<int>(value); }
	template<> inline std::string ToString(Uint8 value) { return ToString<Uint32>(value); }

	template<>
	inline std::string ToString(bool value)
	{
		return value ? "true" : "false";
	}

	template<typename T0, typename T1, typename T2, typename T3>
	inline std::string ToString(const T0& a, const T1& b, const T2& c, const T3& d);

	template<>
	inline std::string ToString(WgOrigo value)
	{
		switch( value )
		{
			case WgOrigo::NorthWest:
				return "northwest";
			case WgOrigo::North:
				return "north";
			case WgOrigo::NorthEast:
				return "northeast";
			case WgOrigo::East:
				return "east";
			case WgOrigo::SouthEast:
				return "southeast";
			case WgOrigo::South:
				return "south";
			case WgOrigo::SouthWest:
				return "southwest";
			case WgOrigo::West:
				return "west";
			case WgOrigo::Center:
				return "center";
			default:
				return "undefined";
		}
	}


	template<typename T0, typename T1>
	inline std::string ToString(const T0& a, const T1& b)
	{
		return ToString<T0>(a) + ", " + ToString<T1>(b);
	}

	template<typename T0, typename T1, typename T2>
	inline std::string ToString(const T0& a, const T1& b, const T2& c)
	{
		return ToString(a, b) + ", " + ToString<T2>(c);
	}

	template<typename T0, typename T1, typename T2, typename T3>
	inline std::string ToString(const T0& a, const T1& b, const T2& c, const T3& d)
	{
		return ToString(a, b) + ", " + ToString(c, d);
	}

	template<typename T0, typename T1, typename T2, typename T3, typename T4>
	inline std::string ToString(const T0& a, const T1& b, const T2& c, const T3& d, const T4& e)
	{
		return ToString(a, b, c) + ", " + ToString(d, e);
	}

	template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
	inline std::string ToString(const T0& a, const T1& b, const T2& c, const T3& d, const T4& e, const T5& f)
	{
		return ToString(a, b, c, d) + ", " + ToString(e, f);
	}

	template<typename T>
	inline bool FromString(const std::string& str, T& a)
	{
		std::istringstream is(str);
		is >> a;
		std::ws(is);
		return is.eof() == true && is.fail() == false;
	}

	template<>
	inline bool FromString(const std::string& str, Sint8& a)
	{
		int i;
		if(FromString<int>(str, i))
		{
			if(Sint8(i) == i)
			{
				a = i;
				return true;
			}
		}
		return false;
	}

	template<>
	inline bool FromString(const std::string& str, Uint8& a)
	{
		Uint32 i;
		if(FromString<Uint32>(str, i))
		{
			if(Uint8(i) == i)
			{
				a = i;
				return true;
			}
		}
		return false;
	}

	template<>
	inline bool FromString(const std::string& str, bool& a)
	{
		std::string b;
		std::istringstream is(str);
		is >> b;
		if(b == "true")
			a = true;
		else if(b == "false")
			a = false;
		else
		{
			float f = 0;
			FromString<float>(str, f);
			return f != 0;
		}
		return true;
	}

	template<typename T0, typename T1, typename T2, typename T3>
	inline int FromString(const std::string& str, T0& a, T1& b, T2& c, T3& d);

	template<>
	inline bool FromString(const std::string& str, WgOrigo& a)
	{
		if(str.empty() || str == "northwest") a = WgOrigo::NorthWest;
		else if(str == "north") a = WgOrigo::North;
		else if(str == "northeast") a = WgOrigo::NorthEast;
		else if(str == "east") a = WgOrigo::East;
		else if(str == "southeast") a = WgOrigo::SouthEast;
		else if(str == "south") a = WgOrigo::South;
		else if(str == "southwest") a = WgOrigo::SouthWest;
		else if(str == "west") a = WgOrigo::West;
		else if(str == "center") a = WgOrigo::Center;
		else
		{
			a = WgOrigo::Undefined;
			return false;
		}
		return true;
	}


	inline bool 	ToBool(const std::string& value, bool def = false)	{ bool 	 v = def; FromString(value, v); return v; }
	inline Sint8	ToSint8(const std::string& value, Sint8 def = 0)	{ Sint8	 v = def; FromString(value, v); return v; }
	inline Sint16	ToSint16(const std::string& value, Sint16 def = 0)	{ Sint16 v = def; FromString(value, v); return v; }
	inline int		ToSint32(const std::string& value, int def = 0)	{ int v = def; FromString(value, v); return v; }
	inline Sint64	ToSint64(const std::string& value, Sint64 def = 0)	{ Sint64 v = def; FromString(value, v); return v; }
	inline Uint8	ToUint8(const std::string& value, Uint8 def = 0)	{ Uint8	 v = def; FromString(value, v); return v; }
	inline Uint16	ToUint16(const std::string& value, Uint16 def = 0)	{ Uint16 v = def; FromString(value, v); return v; }
	inline Uint32	ToUint32(const std::string& value, Uint32 def = 0)	{ Uint32 v = def; FromString(value, v); return v; }
	inline float	ToFloat(const std::string& value, float def = 0)	{ float	 v = def; FromString(value, v); return v; }

	inline void Tokenize(const std::string& str, std::vector<std::string>& tokens)
	{
		if(str.empty())
			return;
		int i0 = 0;
		int i1 = int(str.find(','));
		do
		{
			tokens.push_back(str.substr(i0, i1 - i0));
			i0 = i1 + 1;
			i1 = int(str.find(',', i0));
		}
		while(i0 > 0);
	}

	template<typename T0, typename T1>
	inline int FromString(const std::string& str, T0& a, T1& b)
	{
		std::vector<std::string> tokens;
		Tokenize(str, tokens); tokens.resize(2);
		if(!FromString<T0>(tokens[0], a)) return 0;
		if(!FromString<T1>(tokens[1], b)) return 1;
		return 2;
	}

	template<typename T0, typename T1, typename T2, typename T3>
	inline int FromString(const std::string& str, T0& a, T1& b, T2& c, T3& d)
	{
		std::vector<std::string> tokens;
		Tokenize(str, tokens); tokens.resize(4);
		if(!FromString<T0>(tokens[0], a)) return 0;
		if(!FromString<T1>(tokens[1], b)) return 1;
		if(!FromString<T2>(tokens[2], c)) return 2;
		if(!FromString<T3>(tokens[3], d)) return 3;
		return 4;
	}

	template<typename T0, typename T1, typename T2, typename T3, typename T4>
	inline int FromString(const std::string& str, T0& a, T1& b, T2& c, T3& d, T4& e)
	{
		std::vector<std::string> tokens;
		Tokenize(str, tokens); tokens.resize(5);
		if(!FromString<T0>(tokens[0], a)) return 0;
		if(!FromString<T1>(tokens[1], b)) return 1;
		if(!FromString<T2>(tokens[2], c)) return 2;
		if(!FromString<T3>(tokens[3], d)) return 3;
		if(!FromString<T4>(tokens[4], e)) return 4;
		return 5;
	}

	template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
	inline int FromString(const std::string& str, T0& a, T1& b, T2& c, T3& d, T4& e, T5& f)
	{
		std::vector<std::string> tokens;
		Tokenize(str, tokens); tokens.resize(6);
		if(!FromString<T0>(tokens[0], a)) return 0;
		if(!FromString<T1>(tokens[1], b)) return 1;
		if(!FromString<T2>(tokens[2], c)) return 2;
		if(!FromString<T3>(tokens[3], d)) return 3;
		if(!FromString<T4>(tokens[4], e)) return 4;
		if(!FromString<T5>(tokens[5], f)) return 5;
		return 6;
	}
}



#endif // WG2_UTIL_DOT_H
