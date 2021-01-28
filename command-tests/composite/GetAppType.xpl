<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :get-app-type command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass">
    <Input>
      <xpl:get-app-type/>
    </Input>
    <Expected>console</Expected>
  </MustSucceed>
  
  <Summary/>
</Root>