<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :get-sa-mode and :set-sa-mode commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/set-get-clear">
    <Input>
      <Before>
        <xpl:get-sa-mode/>
      </Before>
      <xpl:set-sa-mode password="1111111"/>
      <Inside>
        <xpl:get-sa-mode/>
      </Inside>
      <xpl:set-sa-mode enable="false"/>
      <After>
        <xpl:get-sa-mode/>
      </After>
    </Input>
    <Expected>
      <Before>false</Before>
      <Inside>true</Inside>
      <After>false</After>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/disable">
    <Input>
      <xpl:set-sa-mode enable="false"/>
    </Input>
    <Expected/>
  </MustSucceed>
  
  <MustFail name="fail/no-password">
    <Input>
      <xpl:set-sa-mode/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/wrong-password">
    <Input>
      <xpl:set-sa-mode password="letmein"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-enable">
    <Input>
      <xpl:set-sa-mode enable="temporarily"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>