namespace litestl::util {
template <typename T, typename... U>
concept IsAnyOf = (std::same_as<T, U> || ...);
};
