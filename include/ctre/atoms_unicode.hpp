#ifndef CTRE__ATOMS_UNICODE__HPP
#define CTRE__ATOMS_UNICODE__HPP

#include "../ext-unicode-db/singleheader/ext/unicode.hpp"

namespace ctre {

// properties name & value

template <auto... Str> struct property_name { };
template <auto... Str> struct property_value { };

template <size_t Sz> constexpr std::string_view get_string_view(const std::array<char, Sz> & arr) noexcept {
	return std::string_view(arr.data(), arr.size());
}

enum class special_binary_property {
	unknown, any, assigned, ascii
};

constexpr special_binary_property special_binary_property_from_string(std::string_view str) noexcept {
	using namespace std::string_view_literals;
	if (uni::__pronamecomp(str, "any"sv) == 0) {
		return special_binary_property::any;
	} else if (uni::__pronamecomp(str, "assigned"sv) == 0) {
		return special_binary_property::assigned;
	} else if (uni::__pronamecomp(str, "ascii"sv) == 0) {
		return special_binary_property::ascii;
	} else {
		return special_binary_property::unknown;
	}
}

// basic support for binary and type-value properties

template <auto Name> struct binary_property;
template <auto Name, auto Value> struct property;

// unicode TS#18 level 1.2 general_category
template <uni::category Category> struct binary_property<Category> {
	template <typename CharT> inline static constexpr bool match_char(CharT c) noexcept {
		return uni::cp_is<Category>(c);
	}
};

// unicode TS#18 level 1.2 any/assigned/ascii

template <> struct binary_property<special_binary_property::any> {
	template <typename CharT> inline static constexpr bool match_char(CharT c) noexcept {
		return uni::cp_is_valid(c);
	}
};

template <> struct binary_property<special_binary_property::assigned> {
	template <typename CharT> inline static constexpr bool match_char(CharT c) noexcept {
		return uni::cp_is_assigned(c);
	}
};

template <> struct binary_property<special_binary_property::ascii> {
	template <typename CharT> inline static constexpr bool match_char(CharT c) noexcept {
		return uni::cp_is_ascii(c);
	}
};

// unicode TS#18 level 1.2.2

enum class property_type {
	script, script_extension, unknown
};

// unicode TS#18 level 1.2.2

template <uni::script Script> struct binary_property<Script> {
	template <typename CharT> inline static constexpr bool match_char(CharT c) noexcept {
		return uni::cp_script(c) == Script;
	}
};

template <uni::script Script> struct property<property_type::script_extension, Script> {
	template <typename CharT> inline static constexpr bool match_char(CharT c) noexcept {
		for (uni::script sc: uni::cp_script_extensions(c)) {
			if (sc == Script) return true;
		}
		return false;
	}
};

// nonbinary properties

constexpr property_type property_type_from_name(std::string_view str) noexcept {
	using namespace std::string_view_literals;
	if (uni::__pronamecomp(str, "script"sv) == 0 || uni::__pronamecomp(str, "sc"sv) == 0) {
		return property_type::script;
	} else if (uni::__pronamecomp(str, "script_extension"sv) == 0 || uni::__pronamecomp(str, "scx"sv) == 0) {
		return property_type::script_extension;
	} else {
		return property_type::unknown;
	}
}

template <property_type Property> struct property_type_builder {
	template <auto... Value> static constexpr auto get() {
		return ctll::reject{};
	}
};

template <auto... Name> struct property_builder {
	static constexpr std::array<char, sizeof...(Name)> name{static_cast<char>(Name)...};
	static constexpr property_type type = property_type_from_name(get_string_view(name));
	
	using helper = property_type_builder<type>;
	
	template <auto... Value> static constexpr auto get() {
		return helper::template get<Value...>();
	}
};

// unicode TS#18 level 1.2.2 script support

template <> struct property_type_builder<property_type::script> {
	template <auto... Value> static constexpr auto get() {
		constexpr std::array<char, sizeof...(Value)> value{Value...};
		constexpr auto sc = uni::__script_from_string(get_string_view(value));
		if constexpr (sc == uni::script::unknown) {
			return ctll::reject{};
		} else {
			return binary_property<sc>();
		}
	}
};

template <> struct property_type_builder<property_type::script_extension> {
	template <auto... Value> static constexpr auto get() {
		constexpr std::array<char, sizeof...(Value)> value{Value...};
		constexpr auto sc = uni::__script_from_string(get_string_view(value));
		if constexpr (sc == uni::script::unknown) {
			return ctll::reject{};
		} else {
			return property<property_type::script_extension, sc>();
		}
	}
};


}

#endif 
