// QA test functions (T01-T20).
#include <iostream>
#include <string>

#include "validation.h"

// Prints a test result and increments failures when the test did not pass; returns nothing.
void check(const std::string& testName, bool passed, int& failures) {
	if (passed) {
		std::cout << "PASS " << testName << '\n';
	} else {
		std::cout << "FAIL " << testName << '\n';
		failures++;
	}
}

// Checks campaign and multiplayer maze size limits; returns nothing.
void testSizeValidation(int& failures) {
    check("T01 campaign size 9 is rejected", !isValidSize(9, false), failures);
    check("T01 campaign size 51 is rejected", !isValidSize(51, false), failures);
    check("T01 campaign size 10 is accepted", isValidSize(10, false), failures);
    check("T01 campaign size 50 is accepted", isValidSize(50, false), failures);
    check("T01 multiplayer size 19 is rejected", !isValidSize(19, true), failures);
    check("T01 multiplayer size 51 is rejected", !isValidSize(51, true), failures);
    check("T01 multiplayer size 20 is accepted", isValidSize(20, true), failures);
    check("T01 multiplayer size 50 is accepted", isValidSize(50, true), failures);
}

// Checks whole-number parsing and preserves the number after invalid text; returns nothing.
void testWholeNumberParsing(int& failures) {
    int number = 7;
    check("T02 abc is rejected", !parseWholeNumber("abc", number), failures);
    check("T02 abc keeps the old number", number == 7, failures);
    check("T02 empty text is rejected", !parseWholeNumber("", number), failures);
    check("T02 empty text keeps the old number", number == 7, failures);
    check("T02 12a is rejected", !parseWholeNumber("12a", number), failures);
    check("T02 12a keeps the old number", number == 7, failures);
    check("T02 42 is accepted", parseWholeNumber("42", number), failures);
    check("T02 parsed number becomes 42", number == 42, failures);
}

// Prints the test summary and returns failure when any check fails.
int main() {
	int failures = 0;
    testSizeValidation(failures);
    testWholeNumberParsing(failures);

	if (failures == 0) {
		std::cout << "All tests passed\n";
	} else {
		std::cout << failures << " checks failed\n";
	}

	if (failures == 0) {
		return 0;
	}

	return 1;
}
