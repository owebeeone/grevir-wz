// Language features Grevir already uses on host, without libstdc++.

template <typename T>
constexpr T twice(T value) {
  return static_cast<T>(value + value);
}

template <typename... Ts>
constexpr auto sum_fold(Ts... values) {
  return (values + ...);
}

int main() {
  constexpr auto n = twice(static_cast<unsigned int>(3));
  constexpr auto folded = sum_fold(1, 2, 3);
  if constexpr (sizeof(int) == 2) {
    static_assert(n == 6, "twice");
    static_assert(folded == 6, "fold");
  }
  return static_cast<int>(n + folded);
}
