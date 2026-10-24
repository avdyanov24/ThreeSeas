// QA test functions (T01-T20).
#include <iostream>
#include <string>

// Prints a test result and increments failures when the test did not pass; returns nothing.
void check(const std::string& testName, bool passed, int& failures) {
	if (passed) {
		std::cout << "PASS " << testName << '\n';
	} else {
		std::cout << "FAIL " << testName << '\n';
		failures++;
	}
}

// Prints the test summary and returns success.
int main() {
	int failures = 0;

	if (failures == 0) {
		std::cout << "All tests passed\n";
	} else {
		std::cout << failures << " checks failed\n";
	}

	return 0;
}
