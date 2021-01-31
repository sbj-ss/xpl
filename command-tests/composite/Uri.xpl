<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :uri-encode and :uri-escape-param commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/uri">
    <Input>
      <En>
        <xpl:uri-encode>http://microsoft.com</xpl:uri-encode>
      </En>
      <Ru>
        <xpl:uri-encode>http://аквадискотека.рф</xpl:uri-encode>
      </Ru>
    </Input>
    <Expected>
      <En>http://microsoft.com</En>
      <Ru>http://xn--80aaaglgumeb6b8ap.xn--p1ai</Ru>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/param">
    <Input>
      <Legal>
        <xpl:uri-escape-param>abc123</xpl:uri-escape-param>
      </Legal>
      <Illegal>
        <xpl:uri-escape-param>?a=1&amp;c=2</xpl:uri-escape-param>
      </Illegal>
    </Input>
    <Expected>
      <Legal>abc123</Legal>
      <Illegal>%3Fa%3D1%26c%3D2</Illegal>
    </Expected>
  </MustSucceed>

  <Summary/>
</Root>