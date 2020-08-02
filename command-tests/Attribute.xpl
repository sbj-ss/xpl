<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :attribute command
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl" xmlns:ns-a="http://a.example.com">
  <AttrFromInside>
    <Container>
      <xpl:attribute name="ns-a:attr-a">content</xpl:attribute>
    </Container>
  </AttrFromInside>
  <AttrFromInsideWithOwnNS>
    <Container>
      <xpl:attribute xmlns:ns-b="http://b.example.com" name="ns-b:b">content</xpl:attribute>
    </Container>
  </AttrFromInsideWithOwnNS>
  <UnicodeName>
    <Container>
      <xpl:attribute name="фпень">и канём!</xpl:attribute>
    </Container>
  </UnicodeName>
  <TargetedAttr>
    <Container/>
    <Container/>
    <xpl:attribute name="attr" destination="preceding-sibling::Container">content</xpl:attribute>
  </TargetedAttr>
  <Replace>
    <Container kept-attr="old" replaced-attr="old">
      <xpl:attribute name="kept-attr" replace="false">new</xpl:attribute>
      <xpl:attribute name="replaced-attr">new</xpl:attribute>
    </Container>
  </Replace>
  <ForceBlank>
    <Container>
      <xpl:attribute name="skipped"/>
      <xpl:attribute name="forced" forceblank="true"/>
    </Container>
  </ForceBlank>
  <UnknownNSError>
    <Container>
      <xpl:attribute name="ns-c:c">content</xpl:attribute>
    </Container>
  </UnknownNSError>
  <NoNameError>
    <Container>
      <xpl:attribute/>
    </Container>
  </NoNameError>
  <EmptyNameError>
    <Container>
      <xpl:attribute name="">content</xpl:attribute>
    </Container>
  </EmptyNameError>
</Root>
