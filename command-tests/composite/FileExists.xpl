<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :file-exists command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/existing">
    <Input>
      <xpl:file-exists file="FileExists.xpl"/>
    </Input>
    <Expected>true</Expected>
  </MustSucceed>

  <MustSucceed name="pass/non-existing">
    <Input>
      <xpl:file-exists file="Heffalump.xls"/>
    </Input>
    <Expected>false</Expected>
  </MustSucceed>

  <MustSucceed name="pass/abspath">
    <Input>
      <xpl:file-exists abspath="true">
        <xpl:attribute name="file">
          <xpl:get-document-filename abspath="true"/>
        </xpl:attribute>
      </xpl:file-exists>
    </Input>
    <Expected>true</Expected>
  </MustSucceed>

  <MustFail name="fail/missing-file">
    <Input>
      <xpl:file-exists/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-abspath">
    <Input>
      <xpl:file-exists file="x" abspath="partially"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>
  