def testReadOnlyProperty(testcase, property, value):
  property # check if the property is there
  testcase.assertEqual(property, value) # check the value

def testReadWriteProperty(testcase, property, initValue, setValue):
  property # check if the property is there
  testcase.assertEqual(property, initValue) # check the initial value
  property = setValue
  testcase.assertEqual(property, setValue) # check the set value



