<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :return command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <xpl:define name="D">
        <xpl:choose>
          <xpl:when>
            <xpl:test>'<xpl:content/>'='a'</xpl:test>
            <xpl:return>
              <A/>
            </xpl:return>
          </xpl:when>
          <xpl:when>
            <xpl:test>'<xpl:content/>'='b'</xpl:test>
            <xpl:return>
              <B>
                <Content/>
              </B>
            </xpl:return>
          </xpl:when>
        </xpl:choose>
        <C>
          <xpl:content/>
        </C>
      </xpl:define>
      <D>a</D>
      <D>b</D>
      <D>other</D>
    </Input>
    <Expected>
      <A/>
      <B>
        <Content/>
      </B>
      <C>other</C>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/outside-macro">
    <Input>
      <xpl:return/>
    </Input>
  </MustFail>
</Root>