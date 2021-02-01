<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :include command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/local-simple">
    <Input>
      <xpl:include xmlns:ns-b="http://b.com">
        <ns-b:A/>
      </xpl:include>
    </Input>
    <Expected xmlns:ns-b="http://b.com">
      <ns-b:A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/local-select">
    <Input>
      <Src>
        <A attr="1">pine</A>
        <B attr="2">apple</B>
      </Src>
      <Element>
        <xpl:include select="../../Src/A"/>
      </Element>
      <Attributes>
        <xpl:include select="../../Src/*/@attr"/>
      </Attributes>
      <Text>
        <xpl:include select="../../Src/*/text()"/>
      </Text>
    </Input>
    <Expected>
      <Src>
        <A attr="1">pine</A>
        <B attr="2">apple</B>
      </Src>
      <Element>
        <A attr="1">pine</A>
      </Element>
      <Attributes>12</Attributes>
      <Text>pineapple</Text>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/local-tag-name">
    <Input>
      <xpl:include tagname="Included">
        <A>
          <Inner/>
        </A>
        <B>
          <Inner2/>
        </B>
      </xpl:include>
    </Input>
    <Expected>
      <Included>
        <Inner/>
      </Included>
      <Included>
        <Inner2/>
      </Included>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/no-repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:include>
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:include>
      <xpl:include repeat="false">
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:include>
    </Input>
    <Expected>
      <ProcessedA/>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/remove-source">
    <Input>
      <A/>
      <B/>
      <C/>
      <xpl:include select="../B" removesource="true"/>
    </Input>
    <Expected>
      <A/>
      <C/>
      <B/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/xml-file">
    <Input>
      <xpl:include select="/Root/node()" file="include/WellFormed.xml"/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/html-file">
    <Input>
      <xpl:include inputformat="html" select="/*/*[local-name()='body']/*[local-name()='ul']" file="include/Markup.html"/>
    </Input>
    <Expected>
      <ul xmlns="http://www.w3.org/1999/xhtml">
        <li>1</li>
        <li>2</li>
        <li>3</li>
      </ul>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/hex">
    <Input>
      <xpl:include outputformat="hex">123</xpl:include>
    </Input>
    <Expected>313233</Expected>
  </MustSucceed>

  <MustSucceed name="pass/base64">
    <Input>
      <xpl:include outputformat="base64">123</xpl:include>
    </Input>
    <Expected>MTIz</Expected>
  </MustSucceed>

  <MustSucceed name="pass/manual-encoding">
    <Input>
      <xpl:include file="include/Enc-koi8r.txt" inputformat="raw" encoding="koi8-r"/>
    </Input>
    <Expected>Проверка распознавания кодировок</Expected>
  </MustSucceed>

  <MustSucceed name="pass/auto-encoding">
    <Input>
      <xpl:define name="Inc">
        <xpl:element>
          <xpl:attribute name="name">
            <xpl:content/>
          </xpl:attribute>
          <xpl:include inputformat="raw" encoding="auto">
            <xpl:attribute name="file">include/Enc-<xpl:content/>.txt</xpl:attribute>
          </xpl:include>
        </xpl:element>
      </xpl:define>
      <Inc>cp866</Inc>
      <Inc>win1251</Inc>
      <Inc>koi8r</Inc>
      <Inc>utf8</Inc>
      <Inc>utf16be</Inc>
      <Inc>utf16le</Inc>
    </Input>
    <Expected>
      <cp866>Проверка распознавания кодировок</cp866>
      <win1251>Проверка распознавания кодировок</win1251>
      <koi8r>Проверка распознавания кодировок</koi8r>
      <utf8>Проверка распознавания кодировок</utf8>
      <utf16be>Проверка распознавания кодировок</utf16be>
      <utf16le>Проверка распознавания кодировок</utf16le>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/abspath">
    <Input>
      <xpl:include abspath="true" select="/Root/node()">
        <xpl:attribute name="file"><xpl:get-option name="DocRoot"/>/include/WellFormed.xml</xpl:attribute>
      </xpl:include>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/bad-select">
    <Input>
      <xpl:include select="))Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/non-nodeset-select">
    <Input>
      <xpl:include select="2+2"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:include repeat="twice"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-tag-name">
    <Input>
      <xpl:include tagname="#"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-abs-path">
    <Input>
      <xpl:include abspath="mostly"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-input-format">
    <Input>
      <xpl:include inputformat="xls"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-output-format">
    <Input>
      <xpl:include outputformat="psd"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-remove-source">
    <Input>
      <xpl:include removesource="partially"/>
    </Input>
  </MustFail>

  <MustFail name="fail/unsuitable-local-format">
    <Input>
      <xpl:include inputformat="html"/>
    </Input>
  </MustFail>

  <MustFail name="fail/raw-to-xml">
    <Input>
      <xpl:include file="whatever" inputformat="raw" outputformat="xml"/>
    </Input>
  </MustFail>

  <MustFail name="fail/file-not-found">
    <Input>
      <xpl:include file="///"/>
    </Input>
  </MustFail>

  <MustFail name="fail/ill-formed-xml-file">
    <Input>
      <xpl:include file="include/IllFormed.xml"/>
    </Input>
  </MustFail>

  <MustFail name="fail/wrong-encoding">
    <Input>
      <xpl:include file="include/Enc-utf16be.txt" inputformat="raw"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>