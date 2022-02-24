<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :save command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <xpl:save file="save/Simple.xml">
        <A/>
      </xpl:save>
      <xpl:include select="/Root/node()" file="save/Simple.xml"/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/custom-root">
    <Input>
      <xpl:save file="save/CustomRoot.xml" root="ns-a:Root">
        <A/>
      </xpl:save>
      <xpl:include select="/ns-a:Root/node()" file="save/CustomRoot.xml"/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/omit-root">
    <Input>
      <xpl:save file="save/OmitRoot.xml" omitroot="true">
        <A/>
      </xpl:save>
      <xpl:include file="save/OmitRoot.xml"/>
    </Input>
    <Expected>
      <A xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/omit-root-with-select">
    <Input>
      <ns-a:A>text</ns-a:A>
      <xpl:save file="save/OmitRootWithSelect.xml" select="preceding-sibling::ns-a:A" omitroot="true"/>
      <xpl:delete select="preceding-sibling::ns-a:A"/>
      <xpl:include file="save/OmitRootWithSelect.xml"/>
    </Input>
    <Expected>
      <ns-a:A xmlns:ns-a="http://a.example.com">text</ns-a:A>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/abspath">
    <Input>
      <xpl:define name="Path" expand="true"><xpl:get-option name="DocRoot"/>/save/AbsPath.xml</xpl:define>
      <xpl:save abspath="true">
        <xpl:attribute name="file">
          <Path/>
        </xpl:attribute>
        <A/>
      </xpl:save>
      <xpl:include select="/Root/node()" abspath="true">
        <xpl:attribute name="file">
          <Path/>
        </xpl:attribute>
      </xpl:include>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/encoding">
    <Input>
      <xpl:save file="save/Encoding.xml" encoding="cp866">Мама мыла Раму</xpl:save>
      <xpl:comment>encoding is already set in the xml preamble</xpl:comment>
      <xpl:include file="save/Encoding.xml" select="/Root/node()"/>
    </Input>
    <Expected>Мама мыла Раму</Expected>
  </MustSucceed>

  <MustSucceed name="pass/create-destination">
    <Input>
      <xpl:save file="save/a/b/c/CreateDestination.xml" createdestination="true">
        <A/>
      </xpl:save>
      <xpl:include select="/Root/node()" file="save/a/b/c/CreateDestination.xml"/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/selected-content">
    <Input>
      <Src xmlns:ns-b="http://b.com">
        <ns-b:A/>
        <B/>
      </Src>
      <xpl:save file="save/SelectedContent.xml" select="../Src/node()"/>
      <xpl:include select="/Root/node()" file="save/SelectedContent.xml"/>
    </Input>
    <Expected xmlns:ns-b="http://b.com">
      <Src xmlns:ns-b="http://b.com">
        <ns-b:A/>
        <B/>
      </Src>
      <ns-b:A/>
      <B/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/selected-content-omit-root">
    <Input>
      <Src xmlns:ns-b="http://b.com">
        <ns-b:A/>
        <B/>
      </Src>
      <xpl:save file="save/SelectedContent.xml" select="../Src/*[1]"/>
      <xpl:include select="/Root/node()" file="save/SelectedContent.xml"/>    
    </Input>
    <Expected>
      <Src xmlns:ns-b="http://b.com">
        <ns-b:A/>
        <B/>
      </Src>
      <ns-b:A xmlns:ns-b="http://b.com"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/selected-scalar">
    <Input>
      <xpl:save file="save/SelectedScalar.xml" select="2+2"/>
      <xpl:include select="/Root/node()" file="save/SelectedScalar.xml"/>
    </Input>
    <Expected>4</Expected>
  </MustSucceed>

  <MustFail name="fail/missing-file">
    <Input>
      <xpl:save/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-file">
    <Input>
      <xpl:save file="////"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-select">
    <Input>
      <xpl:save file="whatever" select="))Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-format">
    <Input>
      <xpl:save file="whatever" format="pretty"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-abs-path">
    <Input>
      <xpl:save file="whatever" abspath="no idea"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-omit-root">
    <Input>
      <xpl:save file="whatever" omitroot="probably"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-create-destination">
    <Input>
      <xpl:save file="whatever" createdestination="unsure"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-root">
    <Input>
      <xpl:save file="whatever" root="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/omit-root-no-child-nodes">
    <Input>
      <xpl:save file="whatever" omitroot="true"/>
    </Input>
  </MustFail>

  <MustFail name="fail/omit-root-many-child-nodes">
    <Input>
      <xpl:save file="whatever" omitroot="true">
        <A/>
        <B/>
      </xpl:save>
    </Input>
  </MustFail>

  <MustFail name="fail/omit-root-non-element-content">
    <Input>
      <xpl:save file="whatever" omitroot="true">text</xpl:save>
    </Input>
  </MustFail>

  <MustFail name="fail/omit-root-empty-selection">
    <Input>
      <xpl:save file="whatever" omitroot="true" select="/Root/missing"/>
    </Input>
  </MustFail>

  <MustFail name="fail/omit-root-multiple-selection">
    <Input>
      <Src>
        <A/>
        <B/>
      </Src>
      <xpl:save file="whatever" omitroot="true" select="../Src/node()"/>
    </Input>
  </MustFail>

  <MustFail name="fail/omit-root-scalar-selection">
    <Input>
      <xpl:save file="whatever" omitroot="true" select="2+2"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>