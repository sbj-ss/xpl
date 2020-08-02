<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :assert command
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl">
  <Test/>
  <PassBoolean>
    <xpl:assert>2*2=4</xpl:assert>
  </PassBoolean>
  <PassNumber>
    <xpl:assert>2+3</xpl:assert>
  </PassNumber>
  <PassString>
    <xpl:assert>'ok'</xpl:assert>
  </PassString>
  <PassNodeset>
    <xpl:assert>/Root/Test</xpl:assert>
  </PassNodeset>
  <FailBoolean>
    <xpl:assert>2*2=8.83</xpl:assert>
  </FailBoolean>
  <FailNumber>
    <xpl:assert>0</xpl:assert>
  </FailNumber>
  <FailString>
    <xpl:assert>''</xpl:assert>
  </FailString>
  <FailNodeset>
    <xpl:assert>../Nonexistent</xpl:assert>
  </FailNodeset>
  <FailNumberWithMessage>
    <xpl:assert message="custom message">0</xpl:assert>
  </FailNumberWithMessage>
</Root>
