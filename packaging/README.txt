Simple Binaural / シンプルバイノーラル
====================================

Binaural spatializer for headphones: places the input by direction, elevation, distance and room. Audio effect in VST® 3 and CLAP formats for Windows x64 and Linux x86_64.
入力の音を、聞き手のまわりの方向・高さ・距離に置き、ヘッドホンでその位置から聞こえるようにする音声エフェクトです。Windows x64 と Linux x86_64 向けの VST® 3 と CLAP の形式で配布します。

Source code and verification / ソースコードと検証:
  https://github.com/AllegroMoltoV/simple-binaural-audio

Install / 導入
--------------

1. Close your DAW. / DAW を終了します。
2. Windows: copy the folder "VST3\Simple Binaural.vst3" to one of:
   Windows では、「VST3\Simple Binaural.vst3」フォルダーを次のどちらかへコピーします。
     %LOCALAPPDATA%\Programs\Common\VST3\   (only you / 自分だけ)
     C:\Program Files\Common Files\VST3\     (all users, needs administrator / 全ユーザー。管理者権限が必要)
3. Windows, for CLAP hosts: copy "CLAP\Simple Binaural.clap" to one of:
   Windows の CLAP に対応した DAW では、「CLAP\Simple Binaural.clap」を次のどちらかへコピーします。
     %LOCALAPPDATA%\Programs\Common\CLAP\
     C:\Program Files\Common Files\CLAP\
4. Linux: copy the folder "VST3/Simple Binaural.vst3" to ~/.vst3/ and "CLAP/Simple Binaural.clap" to ~/.clap/.
   Linux では、「VST3/Simple Binaural.vst3」フォルダーを ~/.vst3/ へ、「CLAP/Simple Binaural.clap」を ~/.clap/ へコピーします。
5. Start your DAW and rescan plug-ins if needed. / DAW を起動し、必要ならプラグインを再スキャンします。

No installer and no network access. The plug-in does not send any audio or data anywhere.
インストーラーはありません。プラグインは通信しません。音声やデータを外部へ送りません。

Parameters / パラメーター
------------------------

Azimuth    -180 to 180 degrees. 0 is the front, +90 the right. / 水平の方向。0 度が正面、+90 度が右
Elevation  -90 to 90 degrees. / 高さ
Distance   0.25 to 10 m. / 距離
Room       0 to 100 %. Reflections and reverb of a medium room. / 中くらいの部屋の反射と残響の量
Quality    Low, Medium or High (1st, 2nd or 3rd order Ambisonics). / 方向の再現の細かさ

For headphones. The latency is 256 samples and is reported to the DAW.
Supported sample rates: 44.1, 48, 88.2, 96, 176.4 and 192 kHz.
ヘッドホン向けの処理です。レイテンシーは 256 サンプルで、DAW に報告します。
対応するサンプルレートは 44.1、48、88.2、96、176.4、192 kHz です。

Licenses / ライセンス
--------------------

Simple Binaural is released under the MIT License (LICENSE.txt).
Third-party licenses are in THIRD_PARTY_LICENSES: Resonance Audio (Apache-2.0), the SADIE HRTF Database
of the University of York, 2016 (Apache-2.0), Eigen (MPL-2.0, unmodified; source at
https://github.com/qt/qtmultimedia/tree/5d5bb219c326e8ea796643e405ab6ae0a05483e6/src/3rdparty/eigen),
pffft, CLAP, clap-wrapper, {fmt} and the VST 3 SDK.

VST is a registered trademark of Steinberg Media Technologies GmbH.
