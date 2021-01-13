<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :attribute command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/from-inside">
    <Input>
      <Container>
        <xpl:attribute name="ns-a:attr-a">content</xpl:attribute>
      </Container>
    </Input>
    <Expected>
      <Container ns-a:attr-a="content"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/from-inside-with-own-ns">
    <Input>
      <Container>
        <xpl:attribute xmlns:ns-b="http://b.example.com" name="ns-b:b">content</xpl:attribute>
      </Container>
    </Input>
    <Expected>
      <Container xmlns:ns-b="http://b.example.com" ns-b:b="content"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/unicode-name">
    <Input>
      <Container>
        <xpl:attribute name="фпень">и канём!</xpl:attribute>
      </Container>
    </Input>
    <Expected>
      <Container фпень="и канём!"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/targeted-attr">
    <Input>
      <Container/>
      <Container/>
      <xpl:attribute name="attr" destination="preceding-sibling::Container">content</xpl:attribute>
    </Input>
    <Expected>
      <Container attr="content"/>
      <Container attr="content"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/replace">
    <Input>
      <Container kept-attr="old" replaced-attr="old">
        <xpl:attribute name="kept-attr" replace="false">new</xpl:attribute>
        <xpl:attribute name="replaced-attr">new</xpl:attribute>
      </Container>
    </Input>
    <Expected>
      <Container kept-attr="old" replaced-attr="new"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/force-blank">
    <Input>
      <Container>
        <xpl:attribute name="skipped"/>
        <xpl:attribute name="forced" forceblank="true"/>
      </Container>
    </Input>
    <Expected>
      <Container forced=""/>
    </Expected>
  </MustSucceed>

  <MustFail name="error/unknown-ns">
    <Input>
      <Container>
        <xpl:attribute name="ns-c:c">content</xpl:attribute>
      </Container>
    </Input>
  </MustFail>

  <MustFail name="error/no-name">
    <Input>
      <Container>
        <xpl:attribute/>
      </Container>
    </Input>
  </MustFail>

  <MustFail name="error/empty-name">
    <Input>
      <Container>
        <xpl:attribute name="">content</xpl:attribute>
      </Container>
    </Input>
  </MustFail>

  <Summary/>
</Root>
