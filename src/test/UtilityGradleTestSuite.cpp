#include "catch.hpp"

#include "language_packages.h"

#if BUILD_JAVA_LANGUAGE_PACKAGE

#	include "FilePath.h"
#	include "utility.h"
#	include "utilityGradle.h"

#	include <iostream>

TEST_CASE("gradle wrapper detects source directories of simple projects")
{
	try
	{
		std::vector<FilePath> result = utility::gradleGetAllSourceDirectories(
			FilePath(L"data/UtilityGradleTestSuite/simple_gradle_project"), false);

		REQUIRE(utility::containsElement<FilePath>(
			result,
			FilePath(L"data/UtilityGradleTestSuite/simple_gradle_project/src/main/java").makeAbsolute()));

		REQUIRE(!utility::containsElement<FilePath>(
			result,
			FilePath(L"data/UtilityGradleTestSuite/simple_gradle_project/src/test/java").makeAbsolute()));
	}
	catch (std::exception e)
	{
		std::cout << "exception: " << e.what << std::endl;
	}
	catch (...)
	{
		std::cout << "unknown exception" << std::endl;
	}
}

TEST_CASE("gradle wrapper detects source and test directories of simple projects")
{
	std::vector<FilePath> result = utility::gradleGetAllSourceDirectories(
		FilePath(L"data/UtilityGradleTestSuite/simple_gradle_project"), true);

	REQUIRE(utility::containsElement<FilePath>(
		result,
		FilePath(L"data/UtilityGradleTestSuite/simple_gradle_project/src/main/java").makeAbsolute()));

	REQUIRE(utility::containsElement<FilePath>(
		result,
		FilePath(L"data/UtilityGradleTestSuite/simple_gradle_project/src/test/java").makeAbsolute()));
}

#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE
