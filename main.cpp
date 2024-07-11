//
// Created by hooman on 7/9/24.
//

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>

#include <occi.h>

using namespace std;
using namespace std::chrono;

using namespace oracle::occi;

template<typename ratio = milliseconds>
class timer
{
private:
	 system_clock::time_point start;
	 const char * title = "";
	 const char * unit = "ms";
public:
	explicit timer(const char * name) : title(name)
	{
		start = high_resolution_clock::now();
	}

	timer(const char * name, const char * unit) : timer(name)
	{
		this->unit = unit;
	}

	timer(timer&& t) = delete;
	timer(const timer & t) = delete;

	~timer()
	{
		cout << '[' << title << "]: " << duration_cast<ratio>(high_resolution_clock::now() - start).count() << " " << unit << endl;
	}
};

class oracle_client
{
private:
	Environment * env = nullptr;
	Connection * conn = nullptr;

public:
	oracle_client(const string & user, const string & passwd, const string & db)
	{
		timer t("oracle initiation");
		env = Environment::createEnvironment(Environment::DEFAULT);
#if 0 && (OCCI_MAJOR_VERSION > 9)
		env->setCacheSortedFlush(true);
#endif
		try
		{
			timer t2("oracle connection");
			conn = env->createConnection(user, passwd, db);
		}
		catch (SQLException &e)
		{
			cout << e.getMessage() << endl;
		}
	}

	~oracle_client()
	{
		timer t("oracle termination");
		if (conn)
			env->terminateConnection(conn);

		Environment::terminateEnvironment(env);
	}

	vector<tuple<int, string, string, string, string, string, string>> getLogs()
	{
		vector<tuple<int, string, string, string, string, string, string>> res;

		timer t("select log");
		auto stmt = conn->createStatement("select * from logs order by id asc");
		ResultSet * rset = stmt->executeQuery();
		try
		{
			while (rset->next())
			{
				timer<microseconds> emplace("emplace", "mic");
				res.emplace_back(rset->getInt(1), rset->getTimestamp(2).toText("yyyy/mm/dd hh:mi:ss tzh:tzm", 0),
								 rset->getString(3), rset->getString(4), rset->getString(5), rset->getString(6), rset->getString(7));
			}
		}
		catch (SQLException ex)
		{
			cout<<"Exception thrown for displayAllRows"<<endl;
			cout<<"Error number: "<<  ex.getErrorCode() << endl;
			cout<<ex.getMessage() << endl;
		}

		stmt->closeResultSet(rset);
		conn->terminateStatement(stmt);

		auto end = high_resolution_clock::now();

		return res;
	}
};

const string username = "tms";
const string password = "123456";
const string database = /*"connection_name = "*/
						  "(DESCRIPTION = "
						  "   (ADDRESS=(PROTOCOL = TCP)(HOST = 172.16.60.250)(PORT = 1521))"
						  "   (CONNECT_DATA= (SERVICE_NAME = ORCLCDB)"
						  "   (SERVER = DEDICATED))"
						  ")";
//const string database = "TCP://172.16.60.250:1521/ORCLCDB";

int main(int argc, char * argv[])
{
	ios_base::sync_with_stdio(false);
	cin.tie(nullptr);
	cout.tie(nullptr);

	timer t("main");
	oracle_client client(username, password, database);
	for (auto & x : client.getLogs())
	{
//		size_t len = tuple_size<decltype(x)>::value;
		cout << get<0>(x) << ", " << get<1>(x) << ", " << get<2>(x) << ", " << get<3>(x) << ", " << get<4>(x) << ", "
				<< get<5>(x) << ", " << get<6>(x) << endl;
	}

	return 0;
}