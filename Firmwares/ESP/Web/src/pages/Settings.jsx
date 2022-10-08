// Страница "ОБНОВЛЕНИЕ" (центр)

import {h, Component} from "preact";
import Interface from "../components/Interface.jsx";
import IDConfig from "../components/IDConfig.jsx";
import IOs from "../components/IOs.jsx";
import Service from "../components/Service.jsx";
import ActInfo from "../components/ActInfo.jsx";
import Message from "../components/Message.jsx";
import Button from "../components/Button.jsx";
import {stringify} from "querystring";

export default class SettingsMain extends Component {
  render({}) {
    return (
      <div>
        <h3 className="tPan"><Message key="settings.title"/></h3>
        <div>

          <Interface/>
          <IDConfig/>
          <IOs/>
          <Service/>
          <ActInfo/>

          {/* Перезагрузка */}
          <Button className="button w75 blue large" onClick={() => this.context.sendWS("Rst")}><Message
            key="settings.rst"/></Button>

        </div>
      </div>
    );
  }
}
