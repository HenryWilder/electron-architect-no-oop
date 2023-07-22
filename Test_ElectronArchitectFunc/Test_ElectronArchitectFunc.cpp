#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestElectronArchitectFunc
{
	TEST_CLASS(TestElectronArchitectFunc)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			double testValue = ClampDouble(-1.0, 0.0, 1.0);
			Assert::AreEqual(0.0, testValue);
		}
	};
}
