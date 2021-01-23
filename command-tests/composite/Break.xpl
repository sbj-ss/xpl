<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :break command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
 
  <MustSucceed name="pass/simple">
    <Input>
      <A/>
      <xpl:break/>
      <B/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/string">
    <Input>
      <A/>
      <xpl:break point="'here'"/>
      <B/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/boolean">
    <Input>
      <A/>
      <xpl:break point="true()"/>
      <B/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/number">
    <Input>
      <A/>
      <xpl:break point="1.0"/>
      <B/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/number-ascend">
    <Input>
      <Start/>
      <Container>
        <A/>
        <xpl:break point="2.3"/>
        <B/>
      </Container>
      <End/>
    </Input>
    <Expected>
      <Start/>
      <Container>
        <A/>
      </Container>
    </Expected>
  </MustSucceed>
   
  <MustSucceed name="pass/nodeset">
    <Input>
      <Container>
        <A/>
        <xpl:break point="Container"/>
        <B/>
      </Container>
      <C/>
    </Input>
    <Expected>
      <Container>
        <A/>
      </Container>
      <C/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/nodeset-ascend">
    <Input>
      <Outer>
        <Inner>
          <A/>
          <xpl:break point="Outer"/>
          <B/>
        </Inner>
        <C/>
      </Outer>
      <D/>
    </Input>
    <Expected>
      <Outer>
        <Inner>
          <A/>
        </Inner>
      </Outer>
      <D/>
    </Expected>
  </MustSucceed>

  <Summary/>
</Root>