<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :error command
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustFail>
    <Input>
      <xpl:error>user-defined error</xpl:error>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>