// РАССЫЛКА ПАРАМЕТРОВ ПОДКЛЮЧЕНИЯ К СЕТИ

import {h} from "preact";
import Message from "./Message.jsx";
import Button from "./Button.jsx";


const SendAuthorization = ({...rest}) => (
  <div>
    {/* Синхронизация параметров авторизации */}
    <p className="bold"><Message key="saut.title"/></p>
    <Button {...rest}><Message key="saut.perform"/></Button>
  </div>
);

export default SendAuthorization;
