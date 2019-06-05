bool Crypt::isCorrect (uint8_t *input) {
  bool returner = true;
  uint8_t correct[] = {1, 2, 3, 4, 5, 6};
  for (uint8_t i = 0; i < 6; i++) {
    if (input[i] != correct[i])
      returner = false;
  }
  return returner;
}
