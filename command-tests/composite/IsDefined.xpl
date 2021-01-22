<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :is-defined command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/name">
    <Input>
      <xpl:define name="A"/>
      <xpl:define name="ns-a:B"/>
      <def-A>
        <xpl:is-defined name="A"/>
      </def-A>
      <def-ns-a-A>
        <xpl:is-defined name="ns-a:A"/>
      </def-ns-a-A>
      <def-B>
        <xpl:is-defined name="B"/>
      </def-B>
      <def-ns-a-B>
        <xpl:is-defined name="ns-a:B"/>
      </def-ns-a-B>
    </Input>
    <Expected>
      <def-A>true</def-A>
      <def-ns-a-A>false</def-ns-a-A>
      <def-B>false</def-B>
      <def-ns-a-B>true</def-ns-a-B>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/unknown-namespace">
    <Input>
      <xpl:is-defined name="heffalump:A"/>
    </Input>
    <Expected>false</Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/at">
    <Input>
      <Outer>
        <Inner>
          <xpl:define name="A"/>
          <AtInner>
            <xpl:is-defined name="A" at="ancestor::Inner[1]"/>
          </AtInner>
          <AtOuter>
            <xpl:is-defined name="A" at="ancestor::Outer[1]"/>
          </AtOuter>
        </Inner>
      </Outer>
    </Input>
    <Expected>
      <Outer>
        <Inner>
          <AtInner>true</AtInner>
          <AtOuter>false</AtOuter>
        </Inner>
      </Outer>
    </Expected>
  </MustSucceed>
  
  <Summary/>
</Root>