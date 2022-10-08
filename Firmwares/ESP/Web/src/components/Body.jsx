import {h, Component} from "preact";

class Body extends Component {
  constructor() {
    super();
    this.body = document.getElementsByTagName("body")[0];
  }

  componentDidMount() {
    this.removeThemeClass();
    if (this.props.dv)
      this.body.classList.add("dv" + this.props.dv);
    this.updateLoading(this.props.loading);
  }

  componentWillReceiveProps(nextProps) {
    this.removeThemeClass();
    if (nextProps.dv)
      this.body.classList.add("dv" + nextProps.dv);
    this.updateLoading(nextProps.loading);
  }

  removeThemeClass() {
    for (let className of this.body.classList) {
      if (className.startsWith("dv"))
        this.body.classList.remove(className);
    }
  }

  updateLoading(param) {
    if (param === true)
      this.body.classList.add("loading");
    if (param === false)
      this.body.classList.remove("loading");
  }
}

export default Body;
