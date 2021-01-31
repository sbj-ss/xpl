<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :serialize command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/own-content">
    <Input>
      <xpl:serialize>
        <A attr="a">
          <?catch-heffalumps all?>
          <ns-b:B xmlns:ns-b="http://b.com" attr="b">text</ns-b:B>
        </A>
      </xpl:serialize>
    </Input>
    <Expected>&lt;A attr="a"&gt;&lt;?catch-heffalumps all?&gt;&lt;ns-b:B xmlns:ns-b="http://b.com" attr="b"&gt;text&lt;/ns-b:B&gt;&lt;/A&gt;</Expected>
  </MustSucceed>

  <MustSucceed name="pass/selected-content">
    <Input>
      <A>1</A>
      <A>2</A>
      <S>
        <xpl:serialize select="../../A"/>
      </S>
    </Input>
    <Expected>
      <A>1</A>
      <A>2</A>
      <S>&lt;A&gt;1&lt;/A&gt;&lt;A&gt;2&lt;/A&gt;</S>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/bad-select">
    <Input>
      <xpl:serialize select="))Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/scalar-selection">
    <Input>
      <xpl:serialize select="2*2"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>