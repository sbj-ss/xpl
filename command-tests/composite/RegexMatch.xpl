<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :regex-match command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/partial">
    <Input>
      <Match>
        <xpl:regex-match regex="\d+">a123b</xpl:regex-match>
      </Match>
      <NoMatch>
        <xpl:regex-match regex="\d+">abc</xpl:regex-match>
      </NoMatch>
    </Input>
    <Expected>
      <Match>true</Match>
      <NoMatch>false</NoMatch>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/full">
    <Input>
      <Match>
        <xpl:regex-match regex="\d+" fullstring="true">123</xpl:regex-match>
      </Match>
      <NoMatch>
        <xpl:regex-match regex="\d+" fullstring="true">a123b</xpl:regex-match>
      </NoMatch>
    </Input>
    <Expected>
      <Match>true</Match>
      <NoMatch>false</NoMatch>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/ignore-case">
    <Input>
      <Match>
        <xpl:regex-match regex="abc" ignorecase="true">AbC</xpl:regex-match>
      </Match>
      <NoMatch>
        <xpl:regex-match regex="abc">AbC</xpl:regex-match>
      </NoMatch>
    </Input>
    <Expected>
      <Match>true</Match>
      <NoMatch>false</NoMatch>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/no-regex">
    <Input>
      <xpl:regex-match/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-regex">
    <Input>
      <xpl:regex-match regex="*"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-full-string">
    <Input>
      <xpl:regex-match regex="\s" fullstring="mostly"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-ignore-case">
    <Input>
      <xpl:regex-match regex="\w" ignorecase="maybe"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>