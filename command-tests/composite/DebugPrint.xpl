<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :debug-print command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/simple">
    <Input>
      <xpl:debug-print severity="debug">text</xpl:debug-print>
      <xpl:debug-print severity="info">text</xpl:debug-print>
      <xpl:debug-print severity="warning">text</xpl:debug-print>
<!-- we can't call the command with the ERROR severity - it will become a false positive 
      <xpl:debug-print severity="error">text</xpl:debug-print>
-->
    </Input>
    <Expected/>
  </MustSucceed>
  
  <MustFail name="fail/invalid-severity">
    <Input>
      <xpl:debug-print severity="snafu"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>