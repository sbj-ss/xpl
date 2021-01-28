<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :get-attributes command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/simple">
    <Input>
      <xpl:get-attributes>
        <A a="1" ns-a:b="2"/>
      </xpl:get-attributes>
    </Input>
    <Expected>
      <attribute name="a">1</attribute>
      <attribute name="ns-a:b">2</attribute>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/custom-tag">
    <Input>
      <xpl:get-attributes tagname="Attr">
        <A a="1" b="2"/>
      </xpl:get-attributes>
    </Input>
    <Expected>
      <Attr name="a">1</Attr>
      <Attr name="b">2</Attr>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/show-tags">
    <Input>
      <xpl:get-attributes showtags="true">
        <A a="1" b="2"/>
      </xpl:get-attributes>
    </Input>
    <Expected>
      <a>1</a>
      <b>2</b>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/selected-node">
    <Input>
      <Carrier a="1" b="2"/>
      <xpl:get-attributes select="preceding-sibling::Carrier"/>
    </Input>
    <Expected>
      <Carrier a="1" b="2"/>
      <attribute name="a">1</attribute>
      <attribute name="b">2</attribute>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/selected-nothing">
    <Input>
      <xpl:get-attributes select="../Nonexistent"/>
    </Input>
    <Expected/>
  </MustSucceed>
  
  <MustSucceed name="pass/empty-children">
    <Input>
      <xpl:get-attributes/>
    </Input>
    <Expected/>
  </MustSucceed>
  
  <MustSucceed name="pass/own-ns">
    <Input>
      <Outer>
        <xpl:get-attributes xmlns:ns-b="http://b.com" tagname="ns-b:Attr">
          <A a="1" b="2"/>
        </xpl:get-attributes>
      </Outer>
    </Input>
    <Expected>
      <Outer xmlns:ns-b="http://b.com">
        <ns-b:Attr name="a">1</ns-b:Attr>
        <ns-b:Attr name="b">2</ns-b:Attr>
      </Outer>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/owner-ns">
    <Input>
      <xpl:get-attributes showtags="true">
        <ns-b:A xmlns:ns-b="http://b.com" xmlns:ns-c="http://c.com" ns-b:a="1" ns-c:b="2"/>
      </xpl:get-attributes>
    </Input>
    <Expected>
      <ns-b:a xmlns:ns-b="http://b.com" >1</ns-b:a>
      <ns-c:b xmlns:ns-c="http://c.com" >2</ns-c:b>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/no-repeat">
    <Input>
      <xpl:define name="attribute">
        <Processed/>
      </xpl:define>
      <xpl:get-attributes repeat="false">
        <A a="1"/>
      </xpl:get-attributes>
    </Input>
    <Expected>
      <attribute name="a">1</attribute>
    </Expected>
  </MustSucceed>
  
  <MustFail name="fail/bad-select">
    <Input>
      <xpl:get-attributes select="))Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/selected-non-element">
    <Input>
      <Something>text</Something>
      <xpl:get-attributes select="../Something/text()"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/selected-non-nodeset">
    <Input>
      <xpl:get-attributes select="123"/>
    </Input>
  </MustFail>

  <MustFail name="fail/show-tags-and-custom-tag-name">
    <Input>
      <xpl:get-attributes showtags="true" tagname="A"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:get-attributes repeat="only-even"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-show-tags">
    <Input>
      <xpl:get-attributes showtags="some"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-tag-name">
    <Input>
      <xpl:get-attributes tagname="#"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>