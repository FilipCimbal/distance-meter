import axios, { AxiosInstance, AxiosRequestConfig, AxiosResponse } from "axios"

export default class ApiConnector {

    _url: string | null
    http: AxiosInstance

    constructor(url: string | null) {

        const headers: Readonly<Record<string, string | boolean>> = {
            Accept: "application/json",
            "Content-Type": "application/json; charset=utf-8",
            "Access-Control-Allow-Credentials": true,
            "X-Requested-With": "XMLHttpRequest",
          };

        this._url = url
        if (this._url == null)
        {
            //this._url = window.location.host
            this._url = 'http://10.0.0.1'
        }
        this.http = axios.create({
            baseURL: '',
            headers
          })
    }


    async setSpeed(speed: number) {
        const sendData = {data: (speed * 1)}
        console.warn('set speed', this._url + '/speed', sendData)
        return (await this.http.post(this._url + '/speed', sendData)).data
    }

    async getFlow() {
        console.warn('getting', this._url + '/measure')
        return (await this.http.get(this._url + '/measure')).data
    }




}