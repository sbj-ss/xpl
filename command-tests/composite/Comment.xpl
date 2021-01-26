<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :comment command
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass">
    <Input>
      <A/>
      <xpl:comment>
        <xpl:delete select="//A"/>
      </xpl:comment>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>
  
  <Summary/>
</Root>